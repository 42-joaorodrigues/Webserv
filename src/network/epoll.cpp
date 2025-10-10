/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: joao-alm <joao-alm@student.42luxembourg    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/06 14:53:35 by naiqing           #+#    #+#             */
/*   Updated: 2025/10/09 18:00:21 by joao-alm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "connection.hpp"
#include "Socket.hpp"
#include "HttpHandler.hpp"
#include "Logger.hpp"
#include <iostream>
#include <limits>
#include <arpa/inet.h>
#include <sstream>

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

	// Client socket is now set to non-blocking mode

	// Initialize the epoll event for the new connection
	// This will allow the epoll instance to monitor the new connection for incoming data
	initEpollEvent(&event, EPOLLIN, newFd);
	socket.addConnection(newFd, i); // Add the new connection to the socket's connection map

	// Log the new connection
	std::ostringstream oss;
	oss << "New Connection From " << inet_ntoa(addr.sin_addr) << ", Assigned Socket " << newFd;
	Logger::connection(oss.str());

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
        // Process epoll event
        //check if the event is for a listening socket
        if (events[j].events & EPOLLERR || events[j].events & EPOLLHUP)
        {
            // Handle error or hang-up events
            Logger::timeout(events[j].data.fd);
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
                // Handle incoming data on client socket
                if (HttpHandler::handleHttpRequest(events[j].data.fd, socket) < 0)
                {
                    // Close connection on error
                    std::ostringstream oss;
                    oss << "HTTP request handling failed for socket " << events[j].data.fd;
                    Logger::debug(oss.str());
                    close(events[j].data.fd);
                    epoll_ctl(socket.getEpollfd(), EPOLL_CTL_DEL, events[j].data.fd, NULL);
                }
            }
            else
            {
                // Handle non-EPOLLIN event
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

    Logger::serverStart("Server Created. ServerName[localhost] Host[127.0.0.1] Port[8002]");

    //main loop, wait and handle epoll events
    //if no ERROR, loop continues
    while (waitEpoll(socket) != ERROR)
		;

    Logger::serverStop("Webserve stopped.");

    //clean sources
	for (size_t i = 0; i < socket.getSocketnumber(); ++i)
	{
		close(socket.getSocket(i)); // Close each socket
		epoll_ctl(socket.getEpollfd(), EPOLL_CTL_DEL, socket.getSocket(i), NULL); // Remove the socket from the epoll instance
	}
	
	close(socket.getEpollfd()); // Close the epoll file descriptor

    return OK;
}