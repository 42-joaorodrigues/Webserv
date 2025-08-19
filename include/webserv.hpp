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
#include <arpa/inet.h> // For inet_addr
#include <fstream> // std::ifstream
#include <sstream> // std::istringstream

#include "../source/server/server.hpp" // For server class
#include "../source/connection/socket.hpp" // For socket class
#include "../source/config/config.hpp" // For config class

class Socket;
class Server;

#define OK 0
#define ERROR -1
#define MAX_EVENTS 10 // Maximum number of events to handle at once

int		setNonBlocking(int fd);
int		initEpoll(Socket &socket);

#endif