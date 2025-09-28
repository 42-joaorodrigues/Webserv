/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: joao-alm <joao-alm@student.42luxembourg    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/06 14:53:35 by naiqing           #+#    #+#             */
/*   Updated: 2025/09/28 19:01:38 by joao-alm         ###   ########.fr       */
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
    
    // Content length and type for POST requests
    if (req.method == "POST") {
        std::ostringstream contentLength;
        contentLength << req.body.length();
        env["CONTENT_LENGTH"] = contentLength.str();
        env["CONTENT_TYPE"] = "application/x-www-form-urlencoded"; // Default, should be from headers
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
    
    // Clean the URI path (remove query parameters)
    size_t queryPos = requestedPath.find('?');
    if (queryPos != std::string::npos) {
        requestedPath = requestedPath.substr(0, queryPos);
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

int handleHttpRequest(int client_fd, Socket &socket)
{
    char buffer[9192];
    std::string rawRequest;
    Request Req;
    ssize_t buffer_read;

    buffer_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (buffer_read > 0)
    {
        buffer[buffer_read] = '\0';
        rawRequest = std::string(buffer);
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
    else if (buffer_read == 0)
    {
        // Client closed connection
        return ERROR;
    }
    else
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            perror("recv");
            return ERROR;
        }
        return OK; // Would block, try again later
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
                if (handleHttpRequest(events[j].data.fd, socket) < 0)
                {
                    // Close connection on error
                    close(events[j].data.fd);
                    epoll_ctl(socket.getEpollfd(), EPOLL_CTL_DEL, events[j].data.fd, NULL);
                }
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