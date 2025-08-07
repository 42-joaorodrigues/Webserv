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
#include "source/epoll/epoll.hpp"// For epoll functions
#include "source/server/server.hpp" // For server class
#include <vector> // For std::vector


#endif