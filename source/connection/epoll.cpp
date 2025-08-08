/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: naiqing <naiqing@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/06 14:53:35 by naiqing           #+#    #+#             */
/*   Updated: 2025/08/08 11:11:22 by naiqing          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"
#include "socket.hpp"

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
		//EAGAIN and EWOULDBLOCK are not errors, they just mean no more connections to accept
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
    for (int i = 0; i < socket.getNumberOfListeningSockets(); ++i)
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
    int nfds = 0;
    int i = 0;

    //main loop, wait for events
    if ((nfds = epoll_wait(socket.getEpollfd(), events, MAX_EVENTS, -1)) < 0)
    {
        perror("epoll_wait");
        return ERROR;
    }

    for (int j = 0; j < nfds; j++)
    {
        //check if the event is for a listening socket
        if (events[j].events & EPOLLERR || events[j].events & EPOLLHUP || events[j].events & EPOLLIN)
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
		//if this fd is not a listening socket, or stdin, it must be a connection that is ready to connected
		{
			TODO: // Handle the connection ready to be read with http
		}
		
    }
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
	for (int i = 0; i < socket.getSocketnumber(); ++i)
	{
		close(socket.getSocket(i)); // Close each socket
		epoll_ctl(socket.getEpollfd(), EPOLL_CTL_DEL, socket.getSocket(i), NULL); // Remove the socket from the epoll instance
	}
	
	close(socket.getEpollfd()); // Close the epoll file descriptor

    return OK;
}