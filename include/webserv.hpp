#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>

#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in structure
#include <cstdlib> // For EXIT_SUCCESS
#include <unistd.h> // For read
#include <cstring> // For strerror
#include <fcntl.h> // For fcntl
#include <sys/epoll.h> 
#include <stdio.h>
#include <vector> // For std::vector
#include <cerrno> // For errno
#include <map> // For std::map
#include <limits>


#include "../source/server/server.hpp" // For server class
#include "../source/connection/socket.hpp" // For socket class
#include "../source/config/config.hpp" // For config class

class Socket;
class Server;

#define OK 0
#define ERROR -1
#define MAX_EVENTS 10 // Maximum number of events to handle at once

typedef std::vector<int> vector_int; // Vector of integers, used for file descriptors
typedef std::vector<Server> vector_servers; // Vector of Server objects, used for managing multiple servers
typedef std::map<int, int> mapSocket; // Map to associate file descriptors with their corresponding server or socket

int		setNonBlocking(int fd);
int		initEpoll(Socket &socket);

#endif