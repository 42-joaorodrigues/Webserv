/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nacao <nacao@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 11:08:21 by nacao             #+#    #+#             */
/*   Updated: 2025/08/07 16:25:38 by nacao            ###   ########.fr       */
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
		vector_servers	_config;
		vector_int		_socket;
		struct sockaddr_in _addr;// socket address structure to save IP address and port, used with bind()
		

	public:
		Socket(/* args */);
		~Socket();
		Socket(const Socket &other);
		Socket &operator=(const Socket &other);

		int initsocket();
		
		void	setEpollfd(int epollfd);
		int		getEpollfd() const;
		size_t  getNumberOfListeningSockets() const;
		int		getListeningSocket(int index) const;
		int		getSocketnumber() const;
		int		getSocket(int n) const;

		
		int		socketMatch(int fd) const;

		int socketMatch(int fd) const; // Check if the file descriptor matches any of the listening sockets

};

#endif