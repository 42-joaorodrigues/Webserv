/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: naiqing <naiqing@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 15:59:01 by naiqing           #+#    #+#             */
/*   Updated: 2025/08/09 11:11:02 by naiqing          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

int Server::getPort() const
{
	return _port;
}

std::string Server::getIp() const
{
	return _ip;
}

Server::Server(std::string ip, int port) : _ip(ip), _port(port)
{
	(void)ip; // Suppress unused parameter warning
	(void)port; // Suppress unused parameter warning
}

Server::~Server()
{
}

