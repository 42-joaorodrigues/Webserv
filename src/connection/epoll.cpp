/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: joao-alm <joao-alm@student.42luxembourg    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/06 14:53:35 by naiqing           #+#    #+#             */
/*   Updated: 2025/09/27 20:13:23 by joao-alm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "connection.hpp"
#include "Socket.hpp"
#include "Server.hpp"
#include "Response.hpp"
#include "Request.hpp"
#include <sstream> // for std::ostringstream

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
    return " ";
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
        
        Response response;
        
        // Check if HTTP method is supported (example: only GET, POST, DELETE)
        if (Req.method != "GET" && Req.method != "POST" && Req.method != "DELETE")
        {
            // 405 Method Not Allowed
            response.setStatus(405, "Method Not Allowed");
            std::string errorContent = geterrorpage(405, serverid, socket);
            response.setBody(errorContent, "text/html");
        }
        else
        {
            std::string requestedPath = Req.uri;
            if (requestedPath == "/")
                requestedPath = "/index.html";
            std::string filePath = "./www" + requestedPath;
            
            // Try to open and read the file
            std::ifstream file(filePath.c_str());
            if (file.is_open())
            {
                // File exists, read its content
                std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                file.close();
                
                // Check if file is empty (could be a permission issue)
                if (content.empty())
                {
                    // 403 Forbidden - File exists but can't read (permission issue)
                    response.setStatus(403, "Forbidden");
                    std::string errorContent = geterrorpage(403, serverid, socket);
                    response.setBody(errorContent, "text/html");
                }
                else
                {
                    // Determine content type based on file extension
                    std::string contentType = "text/html";
                    if (requestedPath.find(".css") != std::string::npos)
                        contentType = "text/css";
                    else if (requestedPath.find(".js") != std::string::npos)
                        contentType = "application/javascript";
                    else if (requestedPath.find(".jpg") != std::string::npos || requestedPath.find(".jpeg") != std::string::npos)
                        contentType = "image/jpeg";
                    else if (requestedPath.find(".png") != std::string::npos)
                        contentType = "image/png";
                    else if (requestedPath.find(".ico") != std::string::npos)
                        contentType = "image/x-icon";
                    
                    // 200 OK - Success
                    response.setStatus(200, "OK");
                    response.setBody(content, contentType);
                }
            }
            else
            {
                // 404 Not Found - File doesn't exist
                response.setStatus(404, "Not Found");
                std::string errorContent = geterrorpage(404, serverid, socket);
                response.setBody(errorContent, "text/html");
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