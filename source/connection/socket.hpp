/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fsilva-p <fsilva-p@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 11:08:21 by nacao             #+#    #+#             */
/*   Updated: 2025/09/08 16:44:16 by fsilva-p         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "../../include/type.hpp"
#include "../../include/webserv.hpp"

class Socket
{
	private:
		int 			_socketfd;
		int 			_epollfd;
		
		vector_servers				_server;
		vector_int					_socket;
		mapSocket					_connected;
		std::vector<sockaddr_in>	_vectorAddr; // Vector to hold multiple server addresses
		std::vector<socklen_t>		_vectorAddrLen; // Vector to hold the lengths of the addresses

		struct sockaddr_in _addr;// socket address structure to save IP address and port, used with bind()
		

	public:
		Socket(Config &config);
		~Socket();
		Socket(const Socket &other);
		Socket &operator=(const Socket &other);
	
		// Methods
		int		initsocket();
		int		socketMatch(int fd) const;

		//getters and setters
		void			setEpollfd(int epollfd);
		int				getEpollfd() const;
		size_t  		getNumberOfListeningSockets() const;
		int				getListeningSocket(int index) const;
		size_t			getSocketnumber() const;
		int				getSocket(int n) const;
		int				getConnection(int connectionSocket);
		void			addConnection(int fd, int serverId);
		void			setSocket(int newSocket);
		void			setAddress(int port, const char *ip);
		const sockaddr_in		&getAddress(size_t index) const;
		socklen_t		getAddressLength(size_t index) const;
		const Server&	getServer(int serverId) const; // Get server by ID
		

};

#endif