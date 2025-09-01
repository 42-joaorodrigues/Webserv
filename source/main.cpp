/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 15:28:01 by marvin            #+#    #+#             */
/*   Updated: 2025/07/25 15:28:01 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"
#include "Request.hpp"

int main(int argc, char *argv[])
{
	Request req;
	std::cout << "Webserver is starting..." << std::endl;

	std::string raw_header =
		"POST /login HTTP/1.1\r\n"
"Host: example.com\r\n"
"Content-Type: application/x-www-form-urlencoded\r\n"
"Content-Length: 27\r\n"
"\r\n"
"username=testando&password=1234\r\n";

	req.parse(raw_header);

	std::cout << "method" << " " << req.method << std::endl;
	std::cout << "urio" << " " << req.uri << std::endl;
	std::cout << "http version" << " " << req.http_version << std::endl;
	std::cout << "body" << " " << req.body << std::endl;
	std::cout << "headers:" << std::endl;
	for (std::map<std::string, std::string>::iterator it = req.headers.begin(); it != req.headers.end(); ++it) {
    	std::cout << "  " << it->first << ": " << it->second << std::endl;
	}




	try {
		std::string config_file;
		if (argc < 2) {
			config_file = "config/NGINX.conf";
		} else if (argc == 2) {
			config_file = argv[1];
		} else {
			std::cerr << "Usage: ./" << argv[0] << " <config_file>(optional)" << std::endl;
			return 2;
		}

		Config config(config_file);
		config.printConfig();

		Socket socket(config);

		initEpoll(socket);

		return OK;
	} catch (const std::exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		return 1;
	}
}