#include "DirectoryListing.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <sstream>
#include <ctime>

std::string DirectoryListing::generate(const std::string& directoryPath, const std::string& requestUri) {
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