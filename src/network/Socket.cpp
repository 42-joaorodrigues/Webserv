/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: joao-alm <joao-alm@student.42luxembourg    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 11:08:49 by nacao             #+#    #+#             */
/*   Updated: 2025/09/27 20:00:53 by joao-alm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

/* 
 * methods
*/

int		Socket::socketMatch(int fd) const
{
	for (size_t i = 0; i < this->getSocketnumber(); ++i)
	{
		if (fd == this->getSocket(i))
			return i;
	}
	return ERROR;
}

void Socket::setAddress(int port, const char *ip)
{
	this->_addr.sin_family = AF_INET;// AF_INET->Address Family Internet & Set address family to IPv4
	this->_addr.sin_addr.s_addr = inet_addr(ip); // Bind to all available interfaces
	this->_addr.sin_port = htons(port); // Set port number (e.g., 8080) and convert to network byte order big-endian
	memset(this->_addr.sin_zero, '\0', sizeof(this->_addr.sin_zero)); // Clear the rest of the structure
	
	this->_vectorAddr.push_back(this->_addr); // Add the address to the vector of addresses
	this->_vectorAddrLen.push_back(sizeof(this->_addr)); // Add the length of the address to the vector of lengths
}
		

//Transmission Control Protocol



int Socket::initsocket()
{
	int num = 1;

	for (size_t i = 0; i < this->_server.size(); ++i)
	{
		this->_socketfd = socket(AF_INET, SOCK_STREAM, 0);
		if (this->_socketfd < 0)
		{
			perror("socket");
			return ERROR;
		}

		this->setSocket(this->_socketfd); // Set the socket file descriptor in the vector of sockets

		//set the socket  to SO_REUSEADDR option
		// This allows the socket to be bound to an address that is already in use
		if (setsockopt(getSocket(i), SOL_SOCKET, SO_REUSEADDR, &num, sizeof(num)))
		{
			perror("setsockopt");
			close(this->_socketfd);
			return ERROR;
		}
		
		// Set the address and port for the socket
		this->setAddress(this->_server[i].getPort(), this->_server[i].getIp().c_str()); // Set the address and port for the socket
		
		// Bind the socket to the address and port
		//(struct sockaddr*)&sockaddr: pass the address information to the kernel
		if(bind(getSocket(i), (struct sockaddr *)&getAddress(i), (int)getAddressLength(i)) < 0)
		{
			perror("bind");
			return ERROR;
		}
		if (setNonBlocking(this->_socketfd) < 0) // Set the socket to non-blocking mode
		{
			perror("setNonBlocking");
			return ERROR;
		}
		if (listen(this->getSocket(i), MAX_EVENTS) < 0) // Listen for incoming connections on the socket
		{
			perror("listen");
			return ERROR;
		}	
	}
	return OK;
}

/*
 * getter & setter
*/

void Socket::setSocket(int newSocket)
{
	this->_socket.push_back(newSocket);
}

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
	return _server.size();
}

// Returns the number of the listening socket at the index
int		Socket::getListeningSocket(int index) const
{
	return _socket[index];
}

size_t		Socket::getSocketnumber() const
{
	return _socket.size();
}

int	Socket::getSocket(int n) const
{
	return this->_socket[n];
}

int Socket::getConnection(int connectionSocket)
{
	return _connected[connectionSocket];
}

void Socket::addConnection(int fd, int serverId)
{
	_connected[fd] = serverId;
}

const sockaddr_in	 &Socket::getAddress(size_t index) const
{
	return _vectorAddr[index];
}

socklen_t Socket::getAddressLength(size_t index) const
{
	return _vectorAddrLen[index];
}


/*
 * Constructor & Destructor
*/

Socket::Socket(Config &config)
{
	this->_server = config.getServers(); // Get the servers from the config
	(void)config;
	if (this->initsocket() < 0)
	{
		perror("initsocket");
		exit(EXIT_FAILURE);
	}
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

const Server& Socket::getServer(int serverId) const
{
	if (serverId >= 0 && serverId < static_cast<int>(_server.size()))
		return _server[serverId];
	// Return first server as fallback if serverId is invalid
	return _server[0];
}