#include "UploadService.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <cctype>
#include <cstdlib>

std::string UploadService::parseMultipartBoundary(const std::string& contentType) {
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos == std::string::npos) {
        return "";
    }
    
    std::string boundary = contentType.substr(boundaryPos + 9);
    
    // Remove quotes if present
    if (!boundary.empty() && boundary[0] == '"') {
        boundary = boundary.substr(1);
    }
    if (!boundary.empty() && boundary[boundary.length() - 1] == '"') {
        boundary = boundary.substr(0, boundary.length() - 1);
    }
    
    return boundary;
}

std::string UploadService::extractFilename(const std::string& contentDisposition) {
    size_t filenamePos = contentDisposition.find("filename=");
    if (filenamePos == std::string::npos) {
        return "";
    }
    
    std::string filename = contentDisposition.substr(filenamePos + 9);
    
    // Remove quotes
    if (!filename.empty() && filename[0] == '"') {
        filename = filename.substr(1);
    }
    if (!filename.empty() && filename[filename.length() - 1] == '"') {
        filename = filename.substr(0, filename.length() - 1);
    }
    
    // Remove any path information for security
    size_t lastSlash = filename.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        filename = filename.substr(lastSlash + 1);
    }
    
    return filename;
}

std::vector<UploadedFile> UploadService::parseMultipartFormData(const std::string& body, const std::string& boundary) {
    std::vector<UploadedFile> files;
    
    std::string delimiter = "--" + boundary;
    std::string endDelimiter = "--" + boundary + "--";
    
    size_t pos = 0;
    
    while (true) {
        // Find next part
        size_t partStart = body.find(delimiter, pos);
        if (partStart == std::string::npos) {
            break;
        }
        
        partStart += delimiter.length();
        
        // Skip CRLF after delimiter
        if (partStart < body.length() && body[partStart] == '\r') partStart++;
        if (partStart < body.length() && body[partStart] == '\n') partStart++;
        
        // Find end of this part
        size_t partEnd = body.find(delimiter, partStart);
        if (partEnd == std::string::npos) {
            break;
        }
        
        // Extract part content
        std::string part = body.substr(partStart, partEnd - partStart);
        
        // Find headers/body separator (double CRLF)
        size_t headerEnd = part.find("\r\n\r\n");
        if (headerEnd == std::string::npos) {
            headerEnd = part.find("\n\n");
            if (headerEnd == std::string::npos) {
                pos = partEnd;
                continue;
            }
        }
        
        std::string headers = part.substr(0, headerEnd);
        std::string content = part.substr(headerEnd + (part.find("\r\n\r\n") != std::string::npos ? 4 : 2));
        
        // Remove trailing CRLF from content
        while (!content.empty() && (content[content.length() - 1] == '\n' || content[content.length() - 1] == '\r')) {
            content = content.substr(0, content.length() - 1);
        }
        
        // Parse headers
        std::string contentDisposition;
        std::string contentType = "application/octet-stream";
        
        std::istringstream headerStream(headers);
        std::string line;
        while (std::getline(headerStream, line)) {
            // Remove carriage return
            if (!line.empty() && line[line.length() - 1] == '\r') {
                line = line.substr(0, line.length() - 1);
            }
            
            if (line.find("Content-Disposition:") == 0) {
                contentDisposition = line.substr(20);
                // Trim leading space
                while (!contentDisposition.empty() && contentDisposition[0] == ' ') {
                    contentDisposition = contentDisposition.substr(1);
                }
            } else if (line.find("Content-Type:") == 0) {
                contentType = line.substr(13);
                // Trim leading space
                while (!contentType.empty() && contentType[0] == ' ') {
                    contentType = contentType.substr(1);
                }
            }
        }
        
        // Extract filename from Content-Disposition
        std::string filename = extractFilename(contentDisposition);
        
        // Only add if it's a file upload (has filename)
        if (!filename.empty()) {
            UploadedFile file;
            file.filename = filename;
            file.contentType = contentType;
            file.content = content;
            files.push_back(file);
        }
        
        pos = partEnd;
    }
    
    return files;
}

size_t UploadService::parseFileSize(const std::string& sizeStr) {
    if (sizeStr.empty()) {
        return 0;
    }
    
    std::string numStr = sizeStr;
    char unit = 'B';
    
    if (!numStr.empty()) {
        char lastChar = numStr[numStr.length() - 1];
        if (lastChar == 'K' || lastChar == 'k' || lastChar == 'M' || lastChar == 'm' || 
            lastChar == 'G' || lastChar == 'g') {
            unit = std::toupper(lastChar);
            numStr = numStr.substr(0, numStr.length() - 1);
        }
    }
    
    size_t size = std::atoi(numStr.c_str());
    
    switch (unit) {
        case 'K': size *= 1024; break;
        case 'M': size *= 1024 * 1024; break;
        case 'G': size *= 1024 * 1024 * 1024; break;
        default: break;
    }
    
    return size;
}

bool UploadService::isUploadAllowed(const LocationData* location, const Request& req, const Server& server) {
    if (!location || location->upload_store.empty()) {
        return false;
    }
    
    if (req.method != "POST") {
        return false;
    }
    
    // Check Content-Type (try different cases)
    std::map<std::string, std::string>::const_iterator ctIt = req.headers.find("Content-Type");
    if (ctIt == req.headers.end()) {
        ctIt = req.headers.find("content-type");
    }
    if (ctIt == req.headers.end()) {
        return false;
    }
    
    if (ctIt->second.find("multipart/form-data") == std::string::npos) {
        return false;  
    }
    
    // Check body size limit
    ssize_t maxSize = server.getClientMaxBodySize();
    if (maxSize > 0 && req.body.length() > static_cast<size_t>(maxSize)) {
        return false;
    }
    
    return true;
}

bool UploadService::createDirectoryRecursive(const std::string& path) {
    struct stat st;
    
    // Check if directory already exists
    if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        return true;
    }
    
    // Try to create directory
    if (mkdir(path.c_str(), 0755) == 0) {
        return true;
    }
    
    // If failed, try to create parent directory first
    size_t lastSlash = path.find_last_of('/');
    if (lastSlash != std::string::npos && lastSlash > 0) {
        std::string parent = path.substr(0, lastSlash);
        if (createDirectoryRecursive(parent)) {
            return mkdir(path.c_str(), 0755) == 0;
        }
    }
    
    return false;
}

bool UploadService::saveUploadedFile(const UploadedFile& file, const std::string& uploadDir) {
    // Ensure upload directory exists
    if (!createDirectoryRecursive(uploadDir)) {
        return false;
    }
    
    // Generate unique filename if file already exists
    std::string filepath = uploadDir + "/" + file.filename;
    std::string baseName = file.filename;
    std::string extension = "";
    
    size_t dotPos = baseName.find_last_of('.');
    if (dotPos != std::string::npos) {
        extension = baseName.substr(dotPos);
        baseName = baseName.substr(0, dotPos);
    }
    
    int counter = 1;
    while (access(filepath.c_str(), F_OK) == 0) {
        std::ostringstream oss;
        oss << baseName << "_" << counter << extension;
        filepath = uploadDir + "/" + oss.str();
        counter++;
    }
    
    // Write file to disk
    std::ofstream outFile(filepath.c_str(), std::ios::binary);
    if (!outFile.is_open()) {
        return false;
    }
    
    outFile.write(file.content.c_str(), file.content.length());
    outFile.close();
    
    return outFile.good();
}