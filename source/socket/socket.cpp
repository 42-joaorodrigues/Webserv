/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nacao <nacao@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 11:08:49 by nacao             #+#    #+#             */
/*   Updated: 2025/08/07 16:09:06 by nacao            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "socket.hpp"
#include <cerrno>

void	Socket::setEpollfd(int epollfd)
{
	this->_epollfd = epollfd;
}

int Socket::getEpollfd() const 
{
	return this->_epollfd;
}

size_t  Socket::getNumberOfListeningSockets() const
{
	return _config.size();
}

// Returns the number of the listening socket at the index
int		Socket::getListeningSocket(int index) const
{
	return _socket[index];
}

int		Socket::getSocketnumber() const
{
	return _socket.size();
}

int	Socket::getSocket(int n) const
{
	return this->_socket[n];
}

int		Socket::socketMatch(int fd) const
{
	for (size_t i = 0; i < this->getSocketnumber(); ++i)
	{
		if (fd == this->getSocket(i))
			return i;
	}
	return ERROR; // Not found
}
		

//Transmission Control Protocol

int Socket::initsocket()
{
	this->_socketfd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (this->_socketfd < 0) {
		std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
		return -1;
	}

	//Listens on 0.0.0.0:8080 for IPv4 requests on all devices
	this->_addr.sin_family = AF_INET;// AF_INET->Address Family Internet & Set address family to IPv4
	this->_addr.sin_addr.s_addr = INADDR_ANY; // Bind to all available interfaces
	this->_addr.sin_port = htons(8080); // Set port number (e.g., 8080) and convert to network byte order big-endian
	
	// Bind the socket to the address and port
	//(struct sockaddr*)&sockaddr: pass the address information to the kernel
	if(bind(this->_socketfd, (struct sockaddr *)&this->_addr, sizeof(this->_addr)) < 0)
	{
		std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
		close(this->_socketfd);
		return -1;
	}

	// Listen for incoming connections, with a backlog of 100
	// The backlog is the maximum length of the queue of pending connections
	if(listen(this->_socketfd, 100) < 0) 
	{
		std::cerr << "Error listening on socket: " << strerror(errno) << std::endl;
		close(this->_socketfd);
		return -1;
	}

	//acept() is used to accept a connection on a socket
	// It blocks until a connection is made, then returns a new socket file descriptor for the connection
	socklen_t addrlen = sizeof(this->_addr);
	int connection = accept(this->_socketfd, (struct sockaddr *)&this->_addr, &addrlen);
	
	// Check if accept() was successful
	// If it returns -1, an error occurred
	if (connection < 0)
	{
		std::cerr << "Error accepting connection: " << strerror(errno) << std::endl;
		close(this->_socketfd);
		return -1;
	}

	//receive data from the client
	char buffer[100];
	ssize_t bytesRead = read(connection, buffer, 100);
	(void)bytesRead; // Ignore the return value for now
	//print the received message from the client
	std::cout << "The message was: " << buffer << std::endl;

	//send a response back to the client
	std::string response = 
		"HTTP/1.1 200 OK\r\n"
		"Content-Length: 19\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n"
		"Hello from socket!\n";
	send(connection, response.c_str(), response.size(), 0);

	close(connection); // Close the connection socket
	close(this->_socketfd); // Close the socket socket


	return 0; // 
}

Socket::Socket()
{
}

Socket::~Socket()
{
}

Socket::Socket(const Socket &other)
{	
	this->_socketfd = other._socketfd;
	this->_addr = other._addr;
}

Socket &Socket::operator=(const Socket &other)
{
	if (this != &other)
	{
		this->_socketfd = other._socketfd;
		this->_addr = other._addr;
	}
	return *this;
}