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
	std::cout << "Webserver is starting..." << std::endl;


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