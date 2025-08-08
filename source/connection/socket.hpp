/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: naiqing <naiqing@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 11:08:21 by nacao             #+#    #+#             */
/*   Updated: 2025/08/08 13:27:53 by naiqing          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "../../include/webserv.hpp"

class Socket
{
	private:
		int 			_socketfd;
		int 			_epollfd;
		
		vector_servers	_server;
		vector_int		_socket;
		mapSocket		_connected;

		struct sockaddr_in _addr;// socket address structure to save IP address and port, used with bind()
		

	public:
		Socket(/* args */);
		~Socket();
		Socket(const Socket &other);
		Socket &operator=(const Socket &other);
	
		// Methods
		int		initsocket();
		int		socketMatch(int fd) const;

		//getters and setters
		void	setEpollfd(int epollfd);
		int		getEpollfd() const;
		size_t  getNumberOfListeningSockets() const;
		int		getListeningSocket(int index) const;
		int		getSocketnumber() const;
		int		getSocket(int n) const;
		int		getConnection(int connectionSocket);
		void	addConnection(int fd, int serverId);
		void	setSocket(int newSocket);

};

#endif