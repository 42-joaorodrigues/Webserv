/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: joao-alm <joao-alm@student.42luxembourg    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/06 14:53:35 by naiqing           #+#    #+#             */
/*   Updated: 2025/10/01 15:58:05 by joao-alm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "connection.hpp"
#include "Socket.hpp"
#include "Server.hpp"
#include "Response.hpp"
#include "Request.hpp"
#include "LocationMatcher.hpp"
#include "ErrorPage.hpp"
#include "../cgi/CGIHandler.hpp"
#include <sstream> // for std::ostringstream
#include <dirent.h> // for directory operations
#include <sys/stat.h> // for file stat
#include <fstream> // for file I/O
#include <cstdlib> // for mkdirs
#include <unistd.h> // for access

std::string generateDirectoryListing(const std::string& directoryPath, const std::string& requestUri) {
    DIR* dir = opendir(directoryPath.c_str());
    if (!dir) {
        return "";
    }
    
    std::ostringstream html;
    html << "<!DOCTYPE html>\n";
    html << "<html>\n";
    html << "<head>\n";
    html << "    <title>Index of " << requestUri << "</title>\n";
    html << "    <style>\n";
    html << "        body { font-family: Arial, sans-serif; margin: 40px; }\n";
    html << "        h1 { color: #333; border-bottom: 1px solid #ccc; padding-bottom: 10px; }\n";
    html << "        table { width: 100%; border-collapse: collapse; }\n";
    html << "        th, td { text-align: left; padding: 8px 12px; border-bottom: 1px solid #eee; }\n";
    html << "        th { background-color: #f5f5f5; font-weight: bold; }\n";
    html << "        a { text-decoration: none; color: #0066cc; }\n";
    html << "        a:hover { text-decoration: underline; }\n";
    html << "        .file-icon { width: 16px; margin-right: 8px; }\n";
    html << "        .dir { color: #0066cc; font-weight: bold; }\n";
    html << "        .file { color: #333; }\n";
    html << "    </style>\n";
    html << "</head>\n";
    html << "<body>\n";
    html << "    <h1>Index of " << requestUri << "</h1>\n";
    html << "    <table>\n";
    html << "        <tr><th>Name</th><th>Size</th><th>Date Modified</th></tr>\n";
    
    // Add parent directory link if not at root
    if (requestUri != "/") {
        std::string parentUri = requestUri;
        if (parentUri[parentUri.length() - 1] == '/') {
            parentUri = parentUri.substr(0, parentUri.length() - 1);
        }
        size_t lastSlash = parentUri.find_last_of('/');
        if (lastSlash != std::string::npos) {
            parentUri = parentUri.substr(0, lastSlash + 1);
        } else {
            parentUri = "/";
        }
        html << "        <tr><td><a href=\"" << parentUri << "\" class=\"dir\">../</a></td><td>-</td><td>-</td></tr>\n";
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string filename = entry->d_name;
        
        // Skip hidden files and current directory
        if (filename[0] == '.') {
            continue;
        }
        
        std::string fullPath = directoryPath + "/" + filename;
        struct stat fileStat;
        
        if (stat(fullPath.c_str(), &fileStat) == 0) {
            std::string linkUri = requestUri;
            if (linkUri[linkUri.length() - 1] != '/') {
                linkUri += "/";
            }
            linkUri += filename;
            
            if (S_ISDIR(fileStat.st_mode)) {
                // Directory
                html << "        <tr><td><a href=\"" << linkUri << "/\" class=\"dir\">" << filename << "/</a></td>";
                html << "<td>-</td>";
            } else {
                // Regular file
                html << "        <tr><td><a href=\"" << linkUri << "\" class=\"file\">" << filename << "</a></td>";
                html << "<td>" << fileStat.st_size << "</td>";
            }
            
            // Format date
            char dateStr[100];
            struct tm* timeinfo = localtime(&fileStat.st_mtime);
            strftime(dateStr, sizeof(dateStr), "%Y-%m-%d %H:%M", timeinfo);
            html << "<td>" << dateStr << "</td></tr>\n";
        }
    }
    
    closedir(dir);
    
    html << "    </table>\n";
    html << "    <hr>\n";
    html << "    <small>Webserv/1.0</small>\n";
    html << "</body>\n";
    html << "</html>\n";
    
    return html.str();
}

// Check if file has CGI extension
bool isCGIRequest(const std::string& filePath, const LocationData* location) {
    if (!location || location->cgi_extension.empty()) {
        return false;
    }
    
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return false;
    }
    
    std::string extension = filePath.substr(dotPos);
    return extension == location->cgi_extension;
}

// Build CGI environment variables
std::map<std::string, std::string> buildCGIEnvironment(const Request& req, const std::string& scriptPath, const MatchedLocation& matched) {
    std::map<std::string, std::string> env;
    (void)matched;
    // Required CGI environment variables
    env["REQUEST_METHOD"] = req.method;
    env["REQUEST_URI"] = req.uri;
    env["SERVER_NAME"] = "webserv";
    env["SERVER_PORT"] = "8080";
    env["SERVER_PROTOCOL"] = "HTTP/1.1";
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SCRIPT_NAME"] = req.uri;
    env["SCRIPT_FILENAME"] = scriptPath;
    
    // Query string
    size_t queryPos = req.uri.find('?');
    if (queryPos != std::string::npos) {
        env["QUERY_STRING"] = req.uri.substr(queryPos + 1);
        env["SCRIPT_NAME"] = req.uri.substr(0, queryPos);
    } else {
        env["QUERY_STRING"] = "";
    }
    
    // PATH_INFO - for ubuntu_cgi_tester, should be the full request URI
    std::string requestUri = queryPos != std::string::npos ? req.uri.substr(0, queryPos) : req.uri;
    env["PATH_INFO"] = requestUri;
    
    // Content length and type for POST requests
    if (req.method == "POST") {
        std::ostringstream contentLength;
        contentLength << req.body.length();
        env["CONTENT_LENGTH"] = contentLength.str();
        env["CONTENT_TYPE"] = "application/x-www-form-urlencoded"; // Default, should be from headers
        
        // Debug: Print body size info
        std::cout << "DEBUG: Request body size = " << req.body.length() << " bytes" << std::endl;
        std::cout << "DEBUG: CONTENT_LENGTH = " << contentLength.str() << std::endl;
    }
    
    // HTTP headers (convert to CGI format)
    for (std::map<std::string, std::string>::const_iterator it = req.headers.begin(); 
         it != req.headers.end(); ++it) {
        std::string headerName = "HTTP_" + it->first;
        // Convert to uppercase and replace - with _
        for (size_t i = 0; i < headerName.length(); i++) {
            if (headerName[i] == '-') {
                headerName[i] = '_';
            } else {
                headerName[i] = std::toupper(headerName[i]);
            }
        }
        env[headerName] = it->second;
    }
    
    return env;
}

// Structure to hold uploaded file information
struct UploadedFile {
    std::string filename;
    std::string contentType;
    std::string content;
};

// Parse multipart boundary from Content-Type header
std::string parseMultipartBoundary(const std::string& contentType) {
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

// Extract filename from Content-Disposition header
std::string extractFilename(const std::string& contentDisposition) {
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

// Parse multipart/form-data and extract uploaded files
std::vector<UploadedFile> parseMultipartFormData(const std::string& body, const std::string& boundary) {
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

// Parse file size from config (e.g., "5M" -> 5242880 bytes)
size_t parseFileSize(const std::string& sizeStr) {
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

// Check if upload is allowed for this location
bool isUploadAllowed(const LocationData* location, const Request& req, const Server& server) {
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

// Create directory recursively
bool createDirectoryRecursive(const std::string& path) {
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

// Save uploaded file to disk
bool saveUploadedFile(const UploadedFile& file, const std::string& uploadDir) {
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

std::string getRedirectMessage(int code) {
    switch (code) {
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 303: return "See Other";
        case 307: return "Temporary Redirect";
        case 308: return "Permanent Redirect";
        default: return "Redirect";
    }
}

std::string getContentType(const std::string& filepath) {
    if (filepath.find(".css") != std::string::npos)
        return "text/css";
    else if (filepath.find(".js") != std::string::npos)
        return "application/javascript";
    else if (filepath.find(".jpg") != std::string::npos || filepath.find(".jpeg") != std::string::npos)
        return "image/jpeg";
    else if (filepath.find(".png") != std::string::npos)
        return "image/png";
    else if (filepath.find(".ico") != std::string::npos)
        return "image/x-icon";
    else if (filepath.find(".gif") != std::string::npos)
        return "image/gif";
    else if (filepath.find(".svg") != std::string::npos)
        return "image/svg+xml";
    else if (filepath.find(".pdf") != std::string::npos)
        return "application/pdf";
    else if (filepath.find(".json") != std::string::npos)
        return "application/json";
    else if (filepath.find(".xml") != std::string::npos)
        return "application/xml";
    else
        return "text/html";
}

std::string geterrorpage(int errorcode, int serverid, Socket &socket)
{
    std::string errorpagepath;
    const Server& server = socket.getServer(serverid);
    const std::map<int, std::string>& errorPages = server.getErrorPages();
    std::map<int, std::string>::const_iterator it = errorPages.find(errorcode);
    
    if (it != errorPages.end())
    {
        errorpagepath = it->second; // Get the path from config (e.g., "/errors/404/404.html")
        
        // Convert config path to actual file path
        if (!errorpagepath.empty())
        {
            if (errorpagepath[0] == '/')
            {
                // Config path starts with '/', combine with server root
                std::string serverRoot = server.getRoot(); // Should return "./www/default"
                errorpagepath = serverRoot + errorpagepath; // Result: "./www/default/errors/404/404.html"
            }
            // If path doesn't start with '/', it's already a full path
        }
    }
    if (!errorpagepath.empty())
    {
        std::ifstream file(errorpagepath.c_str());
        if (file.is_open())
        {
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();
            std::cout << "Successfully loaded custom error page for " << errorcode << std::endl;
            return content;
        }
        else
        {
            std::cout << "Could not open custom error page: " << errorpagepath << std::endl;
        }
    }
    
    // No custom error page found - use default error page
    ErrorPage errorPageBuilder;
    return errorPageBuilder.getdefaulterrorpage(errorcode);
}

void handleLocationRequest(const Request& req, const MatchedLocation& matched, int serverid, Socket& socket, Response& response) {
    std::string requestedPath = req.uri;
    std::string effectiveRoot = matched.effective_root;
    
    // Check client max body size for ALL requests (not just uploads)
    const Server& server = socket.getServer(serverid);
    ssize_t maxSize = server.getClientMaxBodySize();
    if (maxSize > 0 && req.body.length() > static_cast<size_t>(maxSize)) {
        std::cout << "DEBUG: Request body size " << req.body.length() << " exceeds limit " << maxSize << std::endl;
        response.setStatus(413, "Payload Too Large");
        std::string errorContent = geterrorpage(413, serverid, socket);
        response.setBody(errorContent, "text/html");
        return;
    }
    
    // Clean the URI path (remove query parameters)
    size_t queryPos = requestedPath.find('?');
    if (queryPos != std::string::npos) {
        requestedPath = requestedPath.substr(0, queryPos);
    }
    
    // Handle file upload requests
    if (matched.location != NULL && !matched.location->upload_store.empty()) {
        // Get server reference to check upload constraints
        const Server& server = socket.getServer(serverid);
        if (isUploadAllowed(matched.location, req, server)) {
            // Extract Content-Type header to get boundary
            std::map<std::string, std::string>::const_iterator ctIt = req.headers.find("Content-Type");
            if (ctIt == req.headers.end()) {
                ctIt = req.headers.find("content-type");
            }
            if (ctIt != req.headers.end()) {
                std::string boundary = parseMultipartBoundary(ctIt->second);
                if (!boundary.empty()) {
                    // Parse uploaded files
                    std::vector<UploadedFile> uploadedFiles = parseMultipartFormData(req.body, boundary);
                    
                    // Save each uploaded file
                    int successCount = 0;
                    int failureCount = 0;
                    std::ostringstream responseMessage;
                    
                    for (size_t i = 0; i < uploadedFiles.size(); i++) {
                        if (saveUploadedFile(uploadedFiles[i], matched.location->upload_store)) {
                            successCount++;
                            responseMessage << "Successfully uploaded: " << uploadedFiles[i].filename << "<br>";
                        } else {
                            failureCount++;
                            responseMessage << "Failed to upload: " << uploadedFiles[i].filename << "<br>";
                        }
                    }
                    
                    // Generate response
                    if (successCount > 0) {
                        std::ostringstream html;
                        html << "<!DOCTYPE html><html><head><title>Upload Results</title></head><body>";
                        html << "<h1>Upload Results</h1>";
                        html << "<p>Successfully uploaded " << successCount << " file(s)</p>";
                        if (failureCount > 0) {
                            html << "<p>Failed to upload " << failureCount << " file(s)</p>";
                        }
                        html << "<hr>" << responseMessage.str();
                        html << "<p><a href=\"" << requestedPath << "\">Back to upload form</a></p>";
                        html << "</body></html>";
                        
                        response.setStatus(200, "OK");
                        response.setBody(html.str(), "text/html");
                    } else {
                        response.setStatus(500, "Internal Server Error");
                        std::string errorContent = geterrorpage(500, serverid, socket);
                        response.setBody(errorContent, "text/html");
                    }
                    return;
                } else {
                    // Invalid multipart boundary
                    response.setStatus(400, "Bad Request");
                    std::string errorContent = geterrorpage(400, serverid, socket);
                    response.setBody(errorContent, "text/html");
                    return;
                }
            } else {
                // No Content-Type header
                response.setStatus(400, "Bad Request");
                std::string errorContent = geterrorpage(400, serverid, socket);
                response.setBody(errorContent, "text/html");
                return;
            }
        } else if (req.method == "POST") {
            // POST request to upload location but not allowed (size limit, wrong content type, etc.)
            response.setStatus(413, "Payload Too Large");
            std::string errorContent = geterrorpage(413, serverid, socket);
            response.setBody(errorContent, "text/html");
            return;
        }
    }
    
    // Construct the full file path
    std::string filePath;
    
    // Check if this is a directory request (ends with / or matches location path exactly)
    bool isDirectoryRequest = (requestedPath == "/" || 
                              (requestedPath.length() > 0 && requestedPath[requestedPath.length() - 1] == '/') ||
                              (matched.location != NULL && requestedPath == matched.matched_path));
    
    if (isDirectoryRequest) {
        // Try to find an index file
        for (size_t i = 0; i < matched.effective_indexes.size(); i++) {
            std::string indexPath;
            if (requestedPath == "/") {
                indexPath = effectiveRoot + "/" + matched.effective_indexes[i];
            } else {
                // For location matches like /api, look in the location's root
                indexPath = effectiveRoot + "/" + matched.effective_indexes[i];
            }
            std::ifstream indexFile(indexPath.c_str());
            if (indexFile.is_open()) {
                indexFile.close();
                filePath = indexPath;
                break;
            }
        }
        
        // If no index file found, check for autoindex
        if (filePath.empty()) {
            if (matched.location != NULL && matched.location->autoindex) {
                // Generate directory listing
                std::string directoryPath = effectiveRoot;
                if (requestedPath != "/") {
                    // Remove location prefix from URI and add to effective root
                    std::string pathAfterLocation = requestedPath;
                    if (matched.location != NULL && !matched.matched_path.empty() && matched.matched_path != "/") {
                        if (requestedPath.substr(0, matched.matched_path.length()) == matched.matched_path) {
                            pathAfterLocation = requestedPath.substr(matched.matched_path.length());
                        }
                    }
                    directoryPath += pathAfterLocation;
                }
                
                std::string directoryListing = generateDirectoryListing(directoryPath, requestedPath);
                if (!directoryListing.empty()) {
                    response.setStatus(200, "OK");
                    response.setBody(directoryListing, "text/html");
                    return;
                } else {
                    // Directory doesn't exist or can't be read
                    response.setStatus(404, "Not Found");
                    std::string errorContent = geterrorpage(404, serverid, socket);
                    response.setBody(errorContent, "text/html");
                    return;
                }
            } else {
                // 403 Forbidden - No index file and no autoindex
                response.setStatus(403, "Forbidden");
                std::string errorContent = geterrorpage(403, serverid, socket);
                response.setBody(errorContent, "text/html");
                return;
            }
        }
    } else {
        // This is a file request, construct path correctly
        std::string pathAfterLocation = requestedPath;
        
        // Remove location prefix from URI if this is not the root location
        if (matched.location != NULL && !matched.matched_path.empty() && matched.matched_path != "/") {
            if (requestedPath.substr(0, matched.matched_path.length()) == matched.matched_path) {
                pathAfterLocation = requestedPath.substr(matched.matched_path.length());
            }
        }
        
        filePath = effectiveRoot + pathAfterLocation;
    }
    
    // Check if the path is actually a directory
    struct stat pathStat;
    if (stat(filePath.c_str(), &pathStat) == 0) {
        if (S_ISDIR(pathStat.st_mode)) {
            // This is a directory but was accessed without trailing slash
            // Check if autoindex is enabled for this location
            if (matched.location != NULL && matched.location->autoindex) {
                // Generate directory listing
                std::string directoryListing = generateDirectoryListing(filePath, requestedPath);
                if (!directoryListing.empty()) {
                    response.setStatus(200, "OK");
                    response.setBody(directoryListing, "text/html");
                    return;
                } else {
                    // Directory can't be read
                    response.setStatus(403, "Forbidden");
                    std::string errorContent = geterrorpage(403, serverid, socket);
                    response.setBody(errorContent, "text/html");
                    return;
                }
            } else {
                // Try to find an index file in the directory
                for (size_t i = 0; i < matched.effective_indexes.size(); i++) {
                    std::string indexPath = filePath + "/" + matched.effective_indexes[i];
                    std::ifstream indexFile(indexPath.c_str());
                    if (indexFile.is_open()) {
                        indexFile.close();
                        // Redirect to the directory with trailing slash
                        std::string redirectUrl = requestedPath + "/";
                        response.setStatus(301, "Moved Permanently");
                        response.headers["Location"] = redirectUrl;
                        response.setBody("", "text/html");
                        return;
                    }
                }
                
                // No index file found and no autoindex
                response.setStatus(403, "Forbidden");
                std::string errorContent = geterrorpage(403, serverid, socket);
                response.setBody(errorContent, "text/html");
                return;
            }
        }
    }
    
    // Check if this is a CGI request
    if (isCGIRequest(filePath, matched.location)) {
        if (matched.location->cgi_pass.empty()) {
            // CGI extension found but no CGI interpreter configured
            response.setStatus(500, "Internal Server Error");
            std::string errorContent = geterrorpage(500, serverid, socket);
            response.setBody(errorContent, "text/html");
            return;
        }
        
        try {
            // Build CGI environment
            std::map<std::string, std::string> cgiEnv = buildCGIEnvironment(req, filePath, matched);
            
            // Create CGI handler
            CGIHandler cgiHandler(matched.location->cgi_pass, filePath, cgiEnv, req.body);
            
            // Execute CGI script
            std::string cgiOutput = cgiHandler.execute();
            
            // Debug: Print CGI output size info
            std::cout << "DEBUG: CGI output size = " << cgiOutput.length() << " bytes" << std::endl;
            
            // Parse CGI output (headers + body)
            size_t headerEndPos = cgiOutput.find("\r\n\r\n");
            if (headerEndPos == std::string::npos) {
                headerEndPos = cgiOutput.find("\n\n");
            }
            
            if (headerEndPos != std::string::npos) {
                // Parse headers from CGI output
                std::string headers = cgiOutput.substr(0, headerEndPos);
                std::string body = cgiOutput.substr(headerEndPos + (cgiOutput.find("\r\n\r\n") != std::string::npos ? 4 : 2));
                
                // Default content type
                std::string contentType = "text/html";
                
                // Parse CGI headers
                std::istringstream headerStream(headers);
                std::string line;
                while (std::getline(headerStream, line)) {
                    if (!line.empty() && line[line.length() - 1] == '\r') {
                        line = line.substr(0, line.length() - 1);
                    }
                    
                    size_t colonPos = line.find(':');
                    if (colonPos != std::string::npos) {
                        std::string headerName = line.substr(0, colonPos);
                        std::string headerValue = line.substr(colonPos + 1);
                        
                        // Trim whitespace
                        while (!headerValue.empty() && headerValue[0] == ' ') {
                            headerValue = headerValue.substr(1);
                        }
                        
                        if (headerName == "Content-Type" || headerName == "Content-type") {
                            contentType = headerValue;
                        } else {
                            response.headers[headerName] = headerValue;
                        }
                    }
                }
                
                std::cout << "DEBUG: CGI body size (after headers) = " << body.length() << " bytes" << std::endl;
                response.setStatus(200, "OK");
                response.setBody(body, contentType);
            } else {
                // No headers, treat entire output as body
                std::cout << "DEBUG: CGI body size (no headers) = " << cgiOutput.length() << " bytes" << std::endl;
                response.setStatus(200, "OK");
                response.setBody(cgiOutput, "text/html");
            }
            return;
            
        } catch (const std::exception& e) {
            // CGI execution failed
            response.setStatus(500, "Internal Server Error");
            std::string errorContent = geterrorpage(500, serverid, socket);
            response.setBody(errorContent, "text/html");
            return;
        }
    }
    
    // Try to open and read the file
    std::ifstream file(filePath.c_str());
    if (file.is_open()) {
        // File exists, read its content
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        
        // Check if file is empty (could be a permission issue)
        if (content.empty()) {
            // 403 Forbidden - File exists but can't read (permission issue)
            response.setStatus(403, "Forbidden");
            std::string errorContent = geterrorpage(403, serverid, socket);
            response.setBody(errorContent, "text/html");
        } else {
            // Determine content type based on file extension
            std::string contentType = getContentType(filePath);
            
            // 200 OK - Success
            response.setStatus(200, "OK");
            response.setBody(content, contentType);
        }
    } else {
        // 404 Not Found - File doesn't exist
        response.setStatus(404, "Not Found");
        std::string errorContent = geterrorpage(404, serverid, socket);
        response.setBody(errorContent, "text/html");
    }
}

std::string readFullHttpRequest(int client_fd)
{
    std::string request;
    char buffer[4096];
    ssize_t bytes_read;
    bool headers_complete = false;
    size_t content_length = 0;
    size_t headers_end_pos = 0;
    int consecutive_eagain = 0;
    const int max_eagain = 60000;  // 60 seconds timeout for large requests
    
    std::cout << "DEBUG: Starting to read HTTP request from fd " << client_fd << std::endl;

    // First, read until we have complete headers
    while (!headers_complete && consecutive_eagain < max_eagain)
    {
        bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);
        if (bytes_read > 0)
        {
            request.append(buffer, bytes_read);
            consecutive_eagain = 0;
            
            // Check if we have complete headers (look for \r\n\r\n)
            size_t header_end = request.find("\r\n\r\n");
            if (header_end != std::string::npos)
            {
                headers_complete = true;
                headers_end_pos = header_end + 4;
                
                // Extract Content-Length from headers
                size_t content_length_pos = request.find("Content-Length: ");
                if (content_length_pos != std::string::npos && content_length_pos < header_end)
                {
                    content_length_pos += 16;
                    size_t line_end = request.find("\r\n", content_length_pos);
                    if (line_end != std::string::npos)
                    {
                        std::string length_str = request.substr(content_length_pos, line_end - content_length_pos);
                        content_length = static_cast<size_t>(std::atol(length_str.c_str()));
                    }
                }
            }
        }
        else if (bytes_read == 0)
        {
            return "";
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                consecutive_eagain++;
                usleep(1000);  // Reduced sleep time for better responsiveness
                continue;
            }
            else
            {
                return "";
            }
        }
    }

    // Now read the body if there's a Content-Length specified
    if (content_length > 0)
    {
        size_t body_received = request.length() - headers_end_pos;
        consecutive_eagain = 0;
        
        std::cout << "DEBUG: Reading body, expected " << content_length << " bytes" << std::endl;
        
        while (body_received < content_length && consecutive_eagain < max_eagain)
        {
            bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);
            if (bytes_read > 0)
            {
                request.append(buffer, bytes_read);
                body_received += bytes_read;
                consecutive_eagain = 0;
            }
            else if (bytes_read == 0)
            {
                break;
            }
            else
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    consecutive_eagain++;
                    usleep(1000);  // Reduced sleep time for better responsiveness
                    continue;
                }
                else
                {
                    break;
                }
            }
        }
        
        if (content_length > 0) {
            std::cout << "DEBUG: Body reading finished, received " << body_received << " of " << content_length << " bytes" << std::endl;
            if (body_received < content_length) {
                std::cout << "DEBUG: WARNING - Incomplete body read! Missing " << (content_length - body_received) << " bytes" << std::endl;
            }
        }
    }

    std::cout << "DEBUG: Finished reading HTTP request, total size: " << request.length() << " bytes" << std::endl;
    return request;
}

int handleHttpRequest(int client_fd, Socket &socket)
{
    std::string rawRequest;
    Request Req;

    std::cout << "DEBUG: handleHttpRequest called for fd " << client_fd << std::endl;
    rawRequest = readFullHttpRequest(client_fd);
    std::cout << "DEBUG: readFullHttpRequest returned " << rawRequest.length() << " bytes" << std::endl;

    if (!rawRequest.empty())
    {
        int serverid = socket.getConnection(client_fd);
        
        // 400 Bad Request - Request parsing failed
        if (!Req.parse(rawRequest))
        {
            Response errorResponse;
            errorResponse.setStatus(400, "Bad Request");
            std::string errorContent = geterrorpage(400, serverid, socket);
            errorResponse.setBody(errorContent, "text/html");
            std::string responseStr = errorResponse.toString();
            send(client_fd, responseStr.c_str(), responseStr.length(), 0);
            close(client_fd);
            epoll_ctl(socket.getEpollfd(),EPOLL_CTL_DEL, client_fd, NULL);
            return ERROR;
        }
        
        const Server& server = socket.getServer(serverid);
        MatchedLocation matched = LocationMatcher::findMatchingLocation(Req.uri, server);
        Response response;
        
        // Check if redirect is configured for this location
        if (matched.location != NULL && !matched.location->redirect.empty()) {
            // Handle redirect
            std::map<int, std::string>::const_iterator redirect_it = matched.location->redirect.begin();
            response.setStatus(redirect_it->first, getRedirectMessage(redirect_it->first));
            response.headers["Location"] = redirect_it->second;
            response.setBody("", "text/html");
        }
        else {
            // Check if HTTP method is allowed for this location
            if (matched.location != NULL && !matched.location->methods.empty()) {
                bool methodAllowed = false;
                for (size_t i = 0; i < matched.location->methods.size(); i++) {
                    if (matched.location->methods[i] == Req.method) {
                        methodAllowed = true;
                        break;
                    }
                }
                if (!methodAllowed) {
                    // 405 Method Not Allowed - add Allow header with permitted methods
                    response.setStatus(405, "Method Not Allowed");
                    
                    // Build Allow header with all allowed methods for this location
                    std::string allowHeader = "";
                    for (size_t i = 0; i < matched.location->methods.size(); i++) {
                        if (i > 0) allowHeader += ", ";
                        allowHeader += matched.location->methods[i];
                    }
                    response.headers["Allow"] = allowHeader;
                    
                    std::string errorContent = geterrorpage(405, serverid, socket);
                    response.setBody(errorContent, "text/html");
                }
                else {
                    handleLocationRequest(Req, matched, serverid, socket, response);
                }
            }
            else {
                // No method restrictions or no location matched - check global method support
                if (Req.method != "GET" && Req.method != "POST" && Req.method != "DELETE" && Req.method != "HEAD") {
                    // 405 Method Not Allowed - add Allow header with globally supported methods
                    response.setStatus(405, "Method Not Allowed");
                    response.headers["Allow"] = "GET, POST, DELETE, HEAD";
                    std::string errorContent = geterrorpage(405, serverid, socket);
                    response.setBody(errorContent, "text/html");
                }
                else {
                    handleLocationRequest(Req, matched, serverid, socket, response);
                }
            }
        }
        
        // Send the response
        std::string responseStr = response.toString();
        if (send(client_fd, responseStr.c_str(), responseStr.length(), 0) < 0)
        {
            perror("send");
        }
        
        close(client_fd);
        epoll_ctl(socket.getEpollfd(),EPOLL_CTL_DEL, client_fd, NULL);
        return OK;
    }
    else
    {
        // Client closed connection or error reading request
        return ERROR;
    }
}

void initEpollEvent(struct epoll_event *event, uint32_t events, int fd)
{
    memset(event, 0, sizeof(*event)); // Clear the event structure
    event->events = events; // Set the event type (e.g., EPOLLIN, EPOLLOUT)
    event->data.fd = fd; // Associate the file descriptor with the event
}

// Set the fd to non-blocking mode
int setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0); // Get the current file descriptor flags
	if (flags < 0)
	{
		perror("fcntl F_GETFL");
		return ERROR; // Return error if fcntl fails
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) // Set the file descriptor to non-blocking mode
	{
		perror("fcntl F_SETFL O_NONBLOCK");
		return ERROR;
	}
	return OK;
}



int initConnection(Socket &socket, int i)
{   
    struct epoll_event  event;
    struct sockaddr_in  addr; // Structure to hold the address of the incoming connection
    socklen_t           in_len = sizeof(addr); // Length of the address structure
    int                 newFd;

    if ((newFd = accept(socket.getSocket(i), (struct sockaddr *)&addr, &in_len)) < 0) // Accept the new connection
    {
		//EAGAIN and EWOULDBLOCKare not errors, they just mean no more connections to accept
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // No more connections to accept
            return OK;
        }
        else
        {
            perror("accept");
            return ERROR;
        }
    }

	// Set the new socket to non-blocking mode
	if (setNonBlocking(newFd) < 0)
		return ERROR;

	//check is block or not
	int flags = fcntl(newFd, F_GETFL, 0);
	if (flags & O_NONBLOCK) {
		std::cout << "✅ clientFd " << newFd << " is NON-BLOCKING\n";
	} else {
		std::cout << "❌ clientFd " << newFd << " is BLOCKING\n";
	}

	// Initialize the epoll event for the new connection
	// This will allow the epoll instance to monitor the new connection for incoming data
	initEpollEvent(&event, EPOLLIN, newFd);
	socket.addConnection(newFd, i); // Add the new connection to the socket's connection map

	if (epoll_ctl(socket.getEpollfd(), EPOLL_CTL_ADD, newFd, &event) < 0) // Add the new connection to the epoll instance
	{
		perror("epoll_ctl: new connection");
		close(newFd);
		return ERROR;
	}
	return OK;
}

int createSocketEpoll(Socket &socket)
{
    // epoll event structure
    struct epoll_event event;

    //use epoll_create1 to create an epoll instance
    //and save the fd to socket
    //if error occurs, return ERROR
    socket.setEpollfd(epoll_create(MAX_EVENTS));
    if (socket.getEpollfd() < 0)
    {
        perror("epoll_create");
        return ERROR;
    }


    // listen all socket (may have multiple ip/port) add them all to epollfd, make epoll to test automatically the new connections
    for (size_t i = 0; i < socket.getNumberOfListeningSockets(); ++i)
    {
        initEpollEvent(&event, EPOLLIN, socket.getListeningSocket(i));

        // Register the socket with the epoll instance
        if (epoll_ctl(socket.getEpollfd(), EPOLL_CTL_ADD, socket.getListeningSocket(i), &event) < 0)
        {
            perror("epoll_ctl: socket::getListeningSocket");
            return ERROR;
        }
    }

    //if fd is stdin  means can read
    initEpollEvent(&event, EPOLLIN, 0);
    if (epoll_ctl(socket.getEpollfd(), EPOLL_CTL_ADD, 0, &event) < 0)
    {
        perror("epoll_ctl: stdin");
        return ERROR;
    }
    return OK;
    
}

int waitEpoll(Socket &socket)
{
    struct epoll_event  events[MAX_EVENTS]; // Array to hold the events
    int nfds = epoll_wait(socket.getEpollfd(), events, MAX_EVENTS, -1);
    int i = 0;
    
    if (nfds < 0)
    {
        perror("epoll_wait");
        return ERROR;
    }
    
    for (int j = 0; j < nfds; j++)
    {
        std::cout << "DEBUG: Epoll event for fd " << events[j].data.fd << " with events 0x" << std::hex << events[j].events << std::dec << std::endl;
        //check if the event is for a listening socket
        if (events[j].events & EPOLLERR || events[j].events & EPOLLHUP)
        {
            // Handle error or hang-up events
            std::cerr << "Epoll error or hang-up on fd: " << events[j].data.fd << std::endl;
            close(events[j].data.fd);
            return OK;
        }
        // Check if current fd is the socket fd -> means new connection coming
        else if ((i = socket.socketMatch(events[j].data.fd)) >= 0)
        {
            std::cout << "DEBUG: New connection on listening socket " << events[j].data.fd << std::endl;
            if (initConnection(socket, i))
				return ERROR; // Initialize the new connection
        }
		// Check if the event is for stdin (file descriptor 0)
		else if (events[j].data.fd == 0)
		{
			//clean all the content of the stdin, until the \n
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			return ERROR;
		}
		else
        {
            if (events[j].events & EPOLLIN)
            {
                std::cout << "DEBUG: EPOLLIN event for client fd " << events[j].data.fd << std::endl;
                if (handleHttpRequest(events[j].data.fd, socket) < 0)
                {
                    // Close connection on error
                    std::cout << "DEBUG: handleHttpRequest failed for fd " << events[j].data.fd << std::endl;
                    close(events[j].data.fd);
                    epoll_ctl(socket.getEpollfd(), EPOLL_CTL_DEL, events[j].data.fd, NULL);
                }
            }
            else
            {
                std::cout << "DEBUG: Non-EPOLLIN event for client fd " << events[j].data.fd << " (events: 0x" << std::hex << events[j].events << std::dec << ")" << std::endl;
            }
        }
	}
	return OK;
}

int initEpoll(Socket &socket)
{
    //initialize the epoll, subscribe the socket to the epoll instance
    if (createSocketEpoll(socket) < 0)
    {
        return ERROR;
    }

    std::cout << "Webserve started successfully!" << std::endl;

    //main loop, wait and handle epoll events
    //if no ERROR, loop continues
    while (waitEpoll(socket) != ERROR)
		;

    std::cout << "Webserve stopped." << std::endl;

    //clean sources
	for (size_t i = 0; i < socket.getSocketnumber(); ++i)
	{
		close(socket.getSocket(i)); // Close each socket
		epoll_ctl(socket.getEpollfd(), EPOLL_CTL_DEL, socket.getSocket(i), NULL); // Remove the socket from the epoll instance
	}
	
	close(socket.getEpollfd()); // Close the epoll file descriptor

    return OK;
}