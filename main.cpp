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

#include "webserv.hpp"

int main(int argc, char *argv[]) {
	
	(void)argc;
	(void)argv;
	std::cout << "Webserv is starting..." << std::endl;

	server myServer;
	if (myServer.initsocket() < 0)
	{
		std::cerr << "Failed to set up the server." << std::endl;
		return EXIT_FAILURE;
	}
	else
	{
		std::cout << "Server is set up and listening." << std::endl;
	}
	

	return EXIT_SUCCESS;
}