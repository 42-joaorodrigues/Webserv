#ifndef CONNECTION_HPP
#define CONNECTION_HPP

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

class Request;
class Server;
class Socket;

#define OK 0
#define ERROR -1
#define MAX_EVENTS 1000 // Maximum number of events to handle at once
#define LISTEN_BACKLOG 1024 // Maximum pending connections in listen queue

typedef std::vector<int> vector_int; // Vector of integers, used for file descriptors
typedef std::vector<Server> vector_servers; // Vector of Server objects, used for managing multiple servers
typedef std::map<int, int> mapSocket; // Map to associate file descriptors with their corresponding server or socket

int		setNonBlocking(int fd);
int		initEpoll(Socket &socket);
int handleHttpRequest(int client_fd, Socket &socket);
std::string getContentType(const std::string& filepath);
std::string generateResponse(const Request& request, int serverId);

#endif