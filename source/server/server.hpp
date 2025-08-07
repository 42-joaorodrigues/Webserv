#ifndef SERVER_HPP
#define SERVER_HPP

#include "../../webserv.hpp"


class server
{
	private:
		int _socketfd;
		struct sockaddr_in _addr;// Server address structure to save IP address and port, used with bind()
		

	public:
		server(/* args */);
		~server();
		server(const server &other);
		server &operator=(const server &other);

		int initsocket();
		
};





#endif