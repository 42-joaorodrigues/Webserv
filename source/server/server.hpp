#ifndef SERVER_HPP
#define SERVER_HPP

#include "../../include/webserv.hpp"

class Server
{
	private:
		std::string _ip; // IP address for the server
		int _port; // Port number for the server

	public:
		Server(std::string ip, int port);
		~Server();

		int	getPort() const;
		std::string getIp() const;
};






#endif