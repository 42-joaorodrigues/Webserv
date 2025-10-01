#include "CgiUtil.hpp"
#include "Server.hpp"
#include "LocationMatcher.hpp"
#include "Request.hpp"
#include <iostream>
#include <sstream>
#include <cctype>

bool CgiUtil::isCGIRequest(const std::string& filePath, const LocationData* location) {
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

std::map<std::string, std::string> CgiUtil::buildCGIEnvironment(const Request& req, const std::string& scriptPath, const MatchedLocation& matched) {
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
        
        // CGI environment configured for request body
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