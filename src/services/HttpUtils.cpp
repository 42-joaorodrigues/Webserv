#include "HttpUtils.hpp"
#include "Socket.hpp"
#include "Server.hpp"
#include "Logger.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

std::string HttpUtils::getRedirectMessage(int code) {
    switch (code) {
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 303: return "See Other";
        case 307: return "Temporary Redirect";
        case 308: return "Permanent Redirect";
        default: return "Redirect";
    }
}

std::string HttpUtils::getContentType(const std::string& filepath) {
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

std::string HttpUtils::getErrorPage(int errorcode, int serverid, Socket& socket) {
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
            std::ostringstream oss;
            oss << "Loaded custom error page for " << errorcode;
            Logger::debug(oss.str());
            return content;
        }
        else
        {
            Logger::debug("Could not open custom error page: " + errorpagepath);
        }
    }
    
    // No custom error page found - use default error page
    return getDefaultErrorPage(errorcode);
}

std::string HttpUtils::getDefaultErrorPage(int status) {
    switch (status)
    {
        case 400:
            return "<!DOCTYPE html><html><head><title>400 Bad Request</title></head>"
                         "<body><h1>400 Bad Request</h1><p>The server could not understand your request.</p></body></html>";
        case 403:
            return "<!DOCTYPE html><html><head><title>403 Forbidden</title></head>"
                        "<body><h1>403 Request failed due to insufficient permissions</h1></body></html>";
        case 404:
            return "<!DOCTYPE html><html><head><title>404 Page Not Found</title></head>"
                        "<body><h1>404 Page Not Found</h1><p>The requested resource was not found on this server.</p></body></html>";
        case 405: return "<!DOCTYPE html><html><head><title>405 Method Not Allowed</title></head>"
                         "<body><h1>405 Method Not Allowed</h1><p>The request method is not supported for this resource.</p></body></html>";
        case 413: return "<!DOCTYPE html><html><head><title>413 Payload Too Large</title></head>"
                         "<body><h1>413 Payload Too Large</h1><p>The request entity is too large.</p></body></html>";
        case 500: return "<!DOCTYPE html><html><head><title>500 Internal Server Error</title></head>"
                         "<body><h1>500 Internal Server Error</h1><p>The server encountered an internal error.</p></body></html>";
        case 501: return "<!DOCTYPE html><html><head><title>501 Not Implemented</title></head>"
                         "<body><h1>501 Not Implemented</h1><p>This method is not implemented by the server.</p></body></html>";
        case 505: return "<!DOCTYPE html><html><head><title>505 HTTP Version Not Supported</title></head>"
                         "<body><h1>505 HTTP Version Not Supported</h1><p>The server does not support this HTTP version.</p></body></html>";
        default:  return "<!DOCTYPE html><html><head><title>Error</title></head>"
                         "<body><h1>Unknown Error</h1></body></html>";
    }
}