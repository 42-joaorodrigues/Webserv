/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nacao <nacao@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/06 14:53:35 by naiqing           #+#    #+#             */
/*   Updated: 2025/08/07 17:10:47 by nacao            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "epoll.hpp"

void initEpollEvent(struct epoll_event *event, uint32_t events, int fd)
{
    memset(event, 0, sizeof(*event)); // Clear the event structure
    event->events = events; // Set the event type (e.g., EPOLLIN, EPOLLOUT)
    event->data.fd = fd; // Associate the file descriptor with the event
}

void initConnection(Socket &socket, int i)
{   
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
            close(events[j].data.fd); // Close the file descriptor
            return OK;
        }
        // Check if current fd is the socket fd -> means new connection coming
        else if ((i = socket.socketMatch(events[j].data.fd)) >= 0)
        {
            initConnection(socket, i);
            
        }
    }
    

}

int Epoll::initEpoll(Socket &socket)
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
    {
    };

    std::cout << "Webserve stopped." << std::endl;

    //clean sources

    //clean epollfd

    return OK;
}



Epoll::Epoll(/* args */)
{
}

Epoll::~Epoll()
{
}