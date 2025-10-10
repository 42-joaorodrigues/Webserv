#include "HttpHandler.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Socket.hpp"
#include "LocationMatcher.hpp"
#include "Server.hpp"
#include "CGIHandler.hpp"
#include "CgiUtil.hpp"
#include "DirectoryListing.hpp"
#include "UploadService.hpp"
#include <unistd.h>
#include "HttpUtils.hpp"
#include "Logger.hpp"
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>

void HttpHandler::handleLocationRequest(const Request& req, const MatchedLocation& matched, int serverid, Socket& socket, Response& response) {
    std::string requestedPath = req.uri;
    std::string effectiveRoot = matched.effective_root;
    
    // Check client max body size for ALL requests (not just uploads)
    const Server& server = socket.getServer(serverid);
    
    // Use location-specific client_max_body_size if set, otherwise fall back to server-level
    ssize_t maxSize = server.getClientMaxBodySize();
    if (matched.location != NULL && matched.location->client_max_body_size != -1) {
        maxSize = matched.location->client_max_body_size;
    }
    
    // Use actual body size (works for both Content-Length and chunked encoding)
    ssize_t actualBodySize = req.body.length();
    
    // Also check Content-Length header for early detection (before body is fully read)
    std::map<std::string, std::string>::const_iterator clIt = req.headers.find("Content-Length");
    if (clIt == req.headers.end()) {
        clIt = req.headers.find("content-length");
    }
    if (clIt != req.headers.end()) {
        ssize_t headerBodySize = std::strtol(clIt->second.c_str(), NULL, 10);
        actualBodySize = std::max(actualBodySize, headerBodySize);
    }

    if (maxSize > 0 && actualBodySize > maxSize) {
        std::ostringstream oss;
        oss << "Request body size " << req.body.length() << " exceeds limit " << maxSize;
        Logger::warn(oss.str());
        response.setStatus(413, "Payload Too Large");
        std::string errorContent = HttpUtils::getErrorPage(413, serverid, socket);
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
        if (UploadService::isUploadAllowed(matched.location, req, server)) {
            // Extract Content-Type header to get boundary
            std::map<std::string, std::string>::const_iterator ctIt = req.headers.find("Content-Type");
            if (ctIt == req.headers.end()) {
                ctIt = req.headers.find("content-type");
            }
            if (ctIt != req.headers.end()) {
                std::string boundary = UploadService::parseMultipartBoundary(ctIt->second);
                if (!boundary.empty()) {
                    // Parse uploaded files
                    std::vector<UploadedFile> uploadedFiles = UploadService::parseMultipartFormData(req.body, boundary);
                    
                    // Save each uploaded file
                    int successCount = 0;
                    int failureCount = 0;
                    std::ostringstream responseMessage;
                    
                    for (size_t i = 0; i < uploadedFiles.size(); i++) {
                        if (UploadService::saveUploadedFile(uploadedFiles[i], matched.location->upload_store)) {
                            successCount++;
                            responseMessage << "File '" << uploadedFiles[i].filename << "' uploaded successfully.<br>";
                        } else {
                            failureCount++;
                            responseMessage << "Failed to upload file '" << uploadedFiles[i].filename << "'.<br>";
                        }
                    }
                    
                    // Set response
                    if (failureCount == 0) {
                        response.setStatus(200, "OK");
                    } else if (successCount > 0) {
                        response.setStatus(206, "Partial Content");
                    } else {
                        response.setStatus(500, "Internal Server Error");
                        std::string errorContent = HttpUtils::getErrorPage(500, serverid, socket);
                        response.setBody(errorContent, "text/html");
                        return;
                    }
                    
                    std::string htmlResponse = "<!DOCTYPE html><html><head><title>Upload Results</title></head><body>";
                    htmlResponse += "<h1>Upload Results</h1>";
                    htmlResponse += responseMessage.str();
                    htmlResponse += "<p><a href=\"" + requestedPath + "\">Back to upload page</a></p>";
                    htmlResponse += "</body></html>";
                    
                    response.setBody(htmlResponse, "text/html");
                    return;
                } else {
                    // Invalid multipart data
                    response.setStatus(400, "Bad Request");
                    std::string errorContent = HttpUtils::getErrorPage(400, serverid, socket);
                    response.setBody(errorContent, "text/html");
                    return;
                }
            } else {
                // No Content-Type header
                response.setStatus(400, "Bad Request");
                std::string errorContent = HttpUtils::getErrorPage(400, serverid, socket);
                response.setBody(errorContent, "text/html");
                return;
            }
        } else if (req.method == "POST") {
            // POST request to upload location but not allowed (size limit, wrong content type, etc.)
            response.setStatus(413, "Payload Too Large");
            std::string errorContent = HttpUtils::getErrorPage(413, serverid, socket);
            response.setBody(errorContent, "text/html");
            return;
        }
    }
    
    // Handle DELETE requests
    if (req.method == "DELETE") {
        // Calculate the file path to delete (same logic as regular file serving)
        std::string deleteFilePath = LocationMatcher::buildFilesystemPath(requestedPath, matched);
        
        // Check if file exists
        if (access(deleteFilePath.c_str(), F_OK) == 0) {
            // File exists, try to delete it
            if (unlink(deleteFilePath.c_str()) == 0) {
                response.setStatus(200, "OK");
                response.setBody("File deleted successfully", "text/plain");
            } else {
                response.setStatus(500, "Internal Server Error");
                std::string errorContent = HttpUtils::getErrorPage(500, serverid, socket);
                response.setBody(errorContent, "text/html");
            }
        } else {
            response.setStatus(404, "Not Found");
            std::string errorContent = HttpUtils::getErrorPage(404, serverid, socket);
            response.setBody(errorContent, "text/html");
        }
        return;
    }
    
    // Construct the full file path
    std::string filePath;
    
    // Check if this is a directory request (ends with / or matches location path exactly)
    bool isDirectoryRequest = (requestedPath == "/" || 
                              (requestedPath.length() > 0 && requestedPath[requestedPath.length() - 1] == '/') ||
                              requestedPath == matched.matched_path);
    
    if (isDirectoryRequest) {
        // Handle directory requests
        std::string directoryPath = LocationMatcher::buildFilesystemPath(requestedPath, matched);
        
        // Remove trailing slash for file system operations (except root)
        if (directoryPath.length() > 1 && directoryPath[directoryPath.length() - 1] == '/') {
            directoryPath = directoryPath.substr(0, directoryPath.length() - 1);
        }
        
        // Check if directory exists
        struct stat dirStat;
        if (stat(directoryPath.c_str(), &dirStat) != 0 || !S_ISDIR(dirStat.st_mode)) {
            response.setStatus(404, "Not Found");
            std::string errorContent = HttpUtils::getErrorPage(404, serverid, socket);
            response.setBody(errorContent, "text/html");
            return;
        }
        
        // Try to serve index files first
        for (size_t i = 0; i < matched.effective_indexes.size(); i++) {
            std::string indexPath = directoryPath + "/" + matched.effective_indexes[i];
            struct stat indexStat;
            if (stat(indexPath.c_str(), &indexStat) == 0 && S_ISREG(indexStat.st_mode)) {
                // Found index file, serve it
                std::ifstream file(indexPath.c_str(), std::ios::binary);
                if (file.is_open()) {
                    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                    file.close();
                    response.setStatus(200, "OK");
                    response.setBody(content, HttpUtils::getContentType(indexPath));
                    return;
                } else {
                    response.setStatus(404, "Not Found");
					std::string errorContent = HttpUtils::getErrorPage(404, serverid, socket);
					response.setBody(errorContent, "text/html");
					return;
                }
            }
        }
        
        // No index file found, check if autoindex is enabled
        bool autoindex = matched.location ? matched.location->autoindex : false;
        if (autoindex) {
            std::string directoryListing = DirectoryListing::generate(directoryPath, requestedPath);
            if (!directoryListing.empty()) {
                response.setStatus(200, "OK");
                response.setBody(directoryListing, "text/html");
                return;
            } else {
                response.setStatus(404, "Not Found");
				std::string errorContent = HttpUtils::getErrorPage(404, serverid, socket);
				response.setBody(errorContent, "text/html");
				return;
            }
        } else {
            // Directory listing disabled
            response.setStatus(404, "Not Found");
			std::string errorContent = HttpUtils::getErrorPage(404, serverid, socket);
			response.setBody(errorContent, "text/html");
			return;
        }
    } else {
		std::string potentialDirPath = LocationMatcher::buildFilesystemPath(requestedPath, matched);
		struct stat potentialDirStat;
		
		if (stat(potentialDirPath.c_str(), &potentialDirStat) == 0 && S_ISDIR(potentialDirStat.st_mode)) {
			// This is a directory but requested without trailing slash
			// Send 301 redirect to add the trailing slash
			std::string redirectLocation = requestedPath + "/";
			
			response.setStatus(301, "Moved Permanently");
			response.headers["Location"] = redirectLocation;
			response.setBody("", "text/html");
			return;
		}
    
        // Handle file requests
        filePath = LocationMatcher::buildFilesystemPath(requestedPath, matched);
    }
    
    // Check if this is a CGI request
    if (CgiUtil::isCGIRequest(filePath, matched.location)) {
        if (matched.location->cgi_pass.empty()) {
            // CGI extension found but no CGI interpreter configured
            response.setStatus(500, "Internal Server Error");
            std::string errorContent = HttpUtils::getErrorPage(500, serverid, socket);
            response.setBody(errorContent, "text/html");
            return;
        }
        
        try {
            // Build CGI environment
            std::map<std::string, std::string> cgiEnv = CgiUtil::buildCGIEnvironment(req, filePath, matched);
            // Create CGI handler
            CGIHandler cgiHandler(matched.location->cgi_pass, filePath, cgiEnv, req.body);
            
            // Execute CGI script
            std::string cgiOutput = cgiHandler.execute();
            // CGI execution completed
            
            // Parse CGI output (headers + body)
            size_t headerEndPos = cgiOutput.find("\r\n\r\n");
            if (headerEndPos == std::string::npos) {
                headerEndPos = cgiOutput.find("\n\n");
            }
            
            if (headerEndPos != std::string::npos) {
                std::string headers = cgiOutput.substr(0, headerEndPos);
                std::string body = cgiOutput.substr(headerEndPos + (cgiOutput.find("\r\n\r\n") != std::string::npos ? 4 : 2));
                
                // Parse CGI headers
                std::istringstream headerStream(headers);
                std::string line;
                std::string contentType = "text/html"; // Default content type
                
                while (std::getline(headerStream, line)) {
                    if (!line.empty() && line[line.length() - 1] == '\r') {
                        line = line.substr(0, line.length() - 1);
                    }
                    
                    // Handle Content-Type header
                    if (line.find("Content-Type:") == 0) {
                        contentType = line.substr(13);
                        while (!contentType.empty() && contentType[0] == ' ') {
                            contentType = contentType.substr(1);
                        }
                    }
                    // Handle Set-Cookie headers
                    else if (line.find("Set-Cookie:") == 0) {
                        std::string cookieValue = line.substr(11); // Skip "Set-Cookie:"
                        // Remove leading spaces if any
                        while (!cookieValue.empty() && cookieValue[0] == ' ') {
                            cookieValue = cookieValue.substr(1);
                        }
                        response.addCookie(cookieValue);
                    }
                    // Handle other headers
                    else if (line.find(':') != std::string::npos) {
                        size_t colonPos = line.find(':');
                        std::string name = line.substr(0, colonPos);
                        std::string value = line.substr(colonPos + 1);
                        
                        // Remove leading spaces if any
                        while (!value.empty() && value[0] == ' ') {
                            value = value.substr(1);
                        }
                        
                        // Skip headers we handle specially (Content-Type already handled)
                        if (name != "Content-Type") {
                            response.headers[name] = value;
                        }
                    }
                }
                response.setStatus(200, "OK");
                response.setBody(body, contentType);
                return;
            } else {
                // No proper CGI headers found, treat entire output as body
                response.setStatus(200, "OK");
                response.setBody(cgiOutput, "text/html");
                return;
            }
        } catch (const std::exception& e) {
            std::cerr << "CGI execution error: " << e.what() << std::endl;
            response.setStatus(500, "Internal Server Error");
            std::string errorContent = HttpUtils::getErrorPage(500, serverid, socket);
            response.setBody(errorContent, "text/html");
            return;
        }
    }
    
    // Handle regular file requests
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        // File not found
        response.setStatus(404, "Not Found");
        std::string errorContent = HttpUtils::getErrorPage(404, serverid, socket);
        response.setBody(errorContent, "text/html");
        return;
    }
    
    if (!S_ISREG(fileStat.st_mode)) {
        // Not a regular file
        response.setStatus(403, "Forbidden");
        std::string errorContent = HttpUtils::getErrorPage(403, serverid, socket);
        response.setBody(errorContent, "text/html");
        return;
    }
    
    // Check if file is readable
    std::ifstream file(filePath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        response.setStatus(403, "Forbidden");
        std::string errorContent = HttpUtils::getErrorPage(403, serverid, socket);
        response.setBody(errorContent, "text/html");
        return;
    }
    
    // Read and serve the file
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    response.setStatus(200, "OK");
    response.setBody(content, HttpUtils::getContentType(filePath));
}

std::string HttpHandler::readFullHttpRequest(int client_fd) {
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    std::string request;
    int max_attempts = 1000; // Prevent infinite loops
    int attempt = 0;
    bool headers_complete = false;
    size_t expected_content_length = 0;
    size_t header_end_pos = 0;
    
    while (attempt < max_attempts) {
        ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No more data available right now
                if (headers_complete) {
                    // For chunked encoding, don't check expected_content_length (it's 0)
                    // The chunked parsing logic will determine when we're done
                    bool is_chunked = (request.find("Transfer-Encoding:") != std::string::npos || 
                                     request.find("transfer-encoding:") != std::string::npos) &&
                                    (request.find("chunked") != std::string::npos);
                    
                    if (!is_chunked) {
                        // Check if we have all the body data we need for Content-Length
                        size_t current_body_length = request.length() - header_end_pos;
                        if (current_body_length >= expected_content_length) {
                            // We have the complete request
                            break;
                        }
                    }
                    // Wait a bit and try again for more body data
                    usleep(1000); // Sleep for 1ms
                    attempt++;
                    continue;
                } else {
                    // No headers yet, wait and retry
                    usleep(1000); // Sleep for 1ms  
                    attempt++;
                    continue;
                }
            } else {
                perror("recv");
                return "";
            }
        } else if (bytes_received == 0) {
            // Connection closed by client
            break;
        } else {
            // Reset attempt counter when we receive data
            attempt = 0;
            
            // Use append with exact byte count for binary data
            request.append(buffer, bytes_received);
            
            // Check if we have received complete headers
            if (!headers_complete && request.find("\r\n\r\n") != std::string::npos) {
                headers_complete = true;
                header_end_pos = request.find("\r\n\r\n") + 4;
                
                // Check for Transfer-Encoding: chunked first
                bool is_chunked = false;
                size_t te_pos = request.find("Transfer-Encoding:");
                if (te_pos == std::string::npos) {
                    te_pos = request.find("transfer-encoding:");
                }
                if (te_pos != std::string::npos) {
                    size_t line_end = request.find("\r\n", te_pos);
                    if (line_end != std::string::npos) {
                        std::string te_line = request.substr(te_pos, line_end - te_pos);
                        // Convert to lowercase for comparison
                        for (size_t i = 0; i < te_line.size(); ++i) {
                            te_line[i] = std::tolower(te_line[i]);
                        }
                        if (te_line.find("chunked") != std::string::npos) {
                            is_chunked = true;
                        }
                    }
                }
                
                if (is_chunked) {
                    
                    // For chunked encoding, check if we have the complete chunked stream
                    // Look for the final chunk: "0\r\n\r\n" after the headers
                    bool found_final_chunk = false;
                    size_t search_pos = header_end_pos;
                    
                    while (search_pos < request.length()) {
                        size_t chunk_line_end = request.find("\r\n", search_pos);
                        if (chunk_line_end == std::string::npos) {
                            break;
                        }
                        
                        std::string chunk_size_line = request.substr(search_pos, chunk_line_end - search_pos);
                        size_t semicolon = chunk_size_line.find(';');
                        if (semicolon != std::string::npos) {
                            chunk_size_line = chunk_size_line.substr(0, semicolon);
                        }
                        
                        // Parse chunk size
                        unsigned int chunk_size = 0;
                        std::istringstream iss(chunk_size_line);
                        iss >> std::hex >> chunk_size;
                        
                        // Check if this is the final chunk (size 0)
                        if (chunk_size == 0) {
                            // Found final chunk, now look for the trailing CRLF
                            size_t final_crlf = request.find("\r\n", chunk_line_end + 2);
                            if (final_crlf != std::string::npos) {
                                // Complete chunked request found
                                found_final_chunk = true;
                                break;
                            } else {
                                // Final chunk found but no trailing CRLF yet
                                break;
                            }
                        } else {
                            // Regular chunk, skip over chunk data
                            size_t chunk_data_start = chunk_line_end + 2;
                            size_t chunk_data_end = chunk_data_start + chunk_size;
                            if (chunk_data_end + 2 <= request.length()) {
                                // Skip chunk data + trailing CRLF
                                search_pos = chunk_data_end + 2;
                            } else {
                                // Don't have full chunk data yet
                                break;
                            }
                        }
                    }
                    
                    if (found_final_chunk) {
                        break; // Complete chunked request
                    }
                    continue;
                } else {
                    // Look for Content-Length header
                    size_t content_length_pos = request.find("Content-Length:");
                    if (content_length_pos == std::string::npos) {
                        content_length_pos = request.find("content-length:");
                    }
                    
                    if (content_length_pos != std::string::npos) {
                        // Extract content length value
                        size_t value_start = content_length_pos;
                        while (value_start < request.length() && request[value_start] != ':') {
                            value_start++;
                        }
                        value_start++; // Skip the ':'
                        
                        while (value_start < request.length() && request[value_start] == ' ') {
                            value_start++;
                        }
                        
                        size_t value_end = value_start;
                        while (value_end < request.length() && request[value_end] != '\r' && request[value_end] != '\n') {
                            value_end++;
                        }
                        
                        if (value_end > value_start) {
                            std::string content_length_str = request.substr(value_start, value_end - value_start);
                            expected_content_length = std::atoi(content_length_str.c_str());
                        }
                    } else {
                        // No Content-Length header and not chunked, request is complete after headers
                        break;
                    }
                }
            }
            
            // If headers are complete, check if we have all the body (only for Content-Length, not chunked)
            if (headers_complete && expected_content_length > 0) {
                bool is_chunked = (request.find("Transfer-Encoding:") != std::string::npos || 
                                 request.find("transfer-encoding:") != std::string::npos) &&
                                (request.find("chunked") != std::string::npos);
                
                if (!is_chunked) {
                    size_t current_body_length = request.length() - header_end_pos;
                    if (current_body_length >= expected_content_length) {
                        // We have the complete request
                        break;
                    }
                }
            }
        }
    }
    
    return request;
}

int HttpHandler::handleHttpRequest(int client_fd, Socket& socket) {
    // Starting HTTP request handling
    
    std::string rawRequest = readFullHttpRequest(client_fd);
    if (rawRequest.empty()) {
        // Empty request received
        return ERROR;
    }
    
    // Processing HTTP request
    
    Request req;
    if (!req.parse(rawRequest)) {
        Logger::debug("Failed to parse HTTP request");
        Response errorResponse;
        errorResponse.setStatus(400, "Bad Request");
        int serverid = socket.getConnection(client_fd);
        std::string errorContent = HttpUtils::getErrorPage(400, serverid, socket);
        errorResponse.setBody(errorContent, "text/html");
        
        std::string responseStr = errorResponse.toString();
        
        // Send response in chunks for safety
        size_t total_sent = 0;
        const char* data = responseStr.c_str();
        size_t total_size = responseStr.length();
        int retry_count = 0;
        const int MAX_RETRIES = 10;
        
        while (total_sent < total_size) {
            size_t chunk_size = std::min<size_t>(total_size - total_sent, 65536);
            ssize_t sent = send(client_fd, data + total_sent, chunk_size, 0);
            
            if (sent < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    retry_count++;
                    if (retry_count >= MAX_RETRIES) {
                        break;
                    }
                    usleep(100);
                    continue;
                } else {
                    perror("send");
                    break;
                }
            } else if (sent == 0) {
                break;
            } else {
                total_sent += sent;
                retry_count = 0;
            }
        }
        
        if (total_sent == total_size) {
            Logger::response(400, client_fd);
        }
        
        // // Give client time to finish reading before closing socket
        // usleep(1000); // 1ms delay
        
        // Safety: check if socket is still valid before operations
        if (client_fd > 0) {
            close(client_fd);
            epoll_ctl(socket.getEpollfd(),EPOLL_CTL_DEL, client_fd, NULL);
            
            // Decrement connection count (declared in epoll.cpp)
            extern int active_connections;
            if (active_connections > 0) active_connections--;
        }
        
        return OK;
    }
    
    Logger::request(req.method, req.uri, client_fd);
    
    int serverid = socket.getConnection(client_fd);
    if (serverid < 0) {
        Logger::error("Failed to get server ID for client socket");
        return ERROR;
    }
    
    const Server& server = socket.getServer(serverid);
    MatchedLocation matched = LocationMatcher::findMatchingLocation(req.uri, server);
    
    Response response;
    
    // Check if the HTTP method is allowed for this location
    if (matched.location != NULL && !matched.location->methods.empty()) {
        bool methodAllowed = false;
        for (size_t i = 0; i < matched.location->methods.size(); i++) {
            if (matched.location->methods[i] == req.method) {
                methodAllowed = true;
                break;
            }
        }
        
        if (!methodAllowed) {
            if (req.method == "GET" || req.method == "HEAD") {
                response.setStatus(405, "Method Not Allowed");
                std::string errorContent = HttpUtils::getErrorPage(405, serverid, socket);
                response.setBody(errorContent, "text/html");
            } else {
                response.setStatus(405, "Method Not Allowed");
                std::string errorContent = HttpUtils::getErrorPage(405, serverid, socket);
                response.setBody(errorContent, "text/html");
            }
        } else {
            handleLocationRequest(req, matched, serverid, socket, response);
        }
    } else {
        handleLocationRequest(req, matched, serverid, socket, response);
    }
    
    std::string responseStr = response.toString();
    
    // Try simple send first, regardless of size - let OS handle flow control
    ssize_t sent = send(client_fd, responseStr.c_str(), responseStr.length(), MSG_DONTWAIT | MSG_NOSIGNAL);
    
    if (sent == (ssize_t)responseStr.length()) {
        // Complete send - success!
        Logger::response(response.status_code, client_fd);
    } else if (sent > 0 || (sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))) {
        // Partial send or would block - fall back to chunked sending
        size_t already_sent = (sent > 0) ? sent : 0;
        // Send large responses in chunks to avoid partial sends
        size_t total_sent = already_sent;
        const char* data = responseStr.c_str();
        size_t total_size = responseStr.length();
        int consecutive_failures = 0;
        const int MAX_CONSECUTIVE_FAILURES = 100; // Allow more failures for large responses
        
        while (total_sent < total_size) {
            size_t chunk_size = std::min<size_t>(total_size - total_sent, 32768); // 32KB chunks for better flow control
            ssize_t sent = send(client_fd, data + total_sent, chunk_size, 0);
            
            if (sent < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // Socket buffer full, wait for it to drain
                    consecutive_failures++;
                    if (consecutive_failures >= MAX_CONSECUTIVE_FAILURES) {
                        std::cerr << "Too many consecutive send failures, client may be too slow" << std::endl;
                        break;
                    }
                    usleep(10000); // 10ms wait for large responses
                    continue;
                } else {
                    perror("send");
                    break;
                }
            } else if (sent == 0) {
                // Connection closed by client
                std::cerr << "Client closed connection during response send" << std::endl;
                break;
            } else {
                total_sent += sent;
                consecutive_failures = 0; // Reset failure count on successful send
                
                // Progress indicator for very large responses
                if (total_size > 10000000 && total_sent % 1000000 == 0) { // Every 1MB for responses >10MB
                    std::cout << "Sent " << total_sent / 1000000 << "MB of " << total_size / 1000000 << "MB" << std::endl;
                }
            }
        }
        
        if (total_sent == total_size) {
            Logger::response(response.status_code, client_fd);
        } else {
            std::cerr << "Partial send: " << total_sent << " of " << total_size << " bytes" << std::endl;
        }
    } else {
        // Send failed completely
        perror("send failed completely");
    }
    
    // // Give client time to finish reading before closing socket
    // usleep(1000); // 1ms delay
    
    // Safety: check if socket is still valid before operations
    if (client_fd > 0) {
        close(client_fd);
        epoll_ctl(socket.getEpollfd(),EPOLL_CTL_DEL, client_fd, NULL);
        
        // Decrement connection count (declared in epoll.cpp)
        extern int active_connections;
        if (active_connections > 0) active_connections--;
    }
    
    return OK;
}