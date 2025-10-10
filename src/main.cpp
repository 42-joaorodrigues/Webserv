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

#include "Socket.hpp"
#include "Request.hpp"
#include "Logger.hpp"
#include <iostream>
#include <cassert>

int main(int argc, char *argv[])
{
	
	Logger::serverStart("Initializing Servers...");	

	try {
		std::string config_file = "config/webserv.conf";
		if (argc == 2) {
			config_file = argv[1];
		} else if (argc > 2) {
			std::cerr << "Usage: " << argv[0] << " <config_file>(optional)" << std::endl;
			return 2;
		}

		Config config(config_file);
		// config.printConfig();

		Socket socket(config);

		initEpoll(socket);

		return OK;
	} catch (const std::exception& e) {
		Logger::error(std::string("Error: ") + e.what());
		return 1;
	}
}