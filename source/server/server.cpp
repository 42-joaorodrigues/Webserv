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

#include "server.hpp"

void Server::printServer() const {
	std::cout << "Ip: " << _ip << std::endl
			  << "Port: " << _port << std::endl;

	std::cout << "Server Names: ";
	if (_server_names.empty()) {
		std::cout << "(none)";
	} else {
		for (size_t i = 0; i < _server_names.size(); i++) {
			if (i > 0) std::cout << ", ";
			std::cout << _server_names[i];
		}
	}
	std::cout << std::endl;

	std::cout << "Root: " << _root << std::endl;

	std::cout << "Indexes: ";
	if (_indexes.empty()) {
		std::cout << "(none)";
	} else {
		for (size_t i = 0; i < _indexes.size(); i++) {
			if (i > 0) std::cout << ", ";
			std::cout << _indexes[i];
		}
	}
	std::cout << std::endl;

	std::cout << "Error Pages:" << std::endl;
	if (_error_pages.empty()) {
		std::cout << " (none)" << std::endl;
	} else {
		for (std::map<int, std::string>::const_iterator it = _error_pages.begin();
			 it != _error_pages.end(); ++it) {
			std::cout << "  Error " << it->first << " -> " << it->second << std::endl;
			 }
	}

	std::cout << "Client Max Body Size: " << _client_max_body_size << " bytes" << std::endl;

	std::cout << "Locations:" << std::endl;
	if (_locations.empty()) {
		std::cout << " (none)" << std::endl;
	} else {
		for (size_t i = 0; i < _locations.size(); i++) {
			const Location& loc = _locations[i];
			std::cout << " Location " << i << ":" << std::endl;

			std::cout << "  Path: " << loc.path << std::endl;

			std::cout << "  Methods: ";
			if (loc.methods.empty()) {
				std::cout << "(none)";
			} else {
				for (size_t j = 0; j < loc.methods.size(); j++) {
					if (j > 0) std::cout << ", ";
					std::cout << loc.methods[j];
				}
			}
			std::cout << std::endl;

			std::cout << "  Root: " << loc.root << std::endl
					  << "  Index: " << loc.index << std::endl
					  << "  Auto Index: " << (loc.autoindex ? "on" : "off") << std::endl
					  << "  Upload Store: " << loc.upload_store << std::endl;

			std::cout << "  Redirect: ";
			if (!loc.redirect.exists) {
				std::cout << "(none)" << std::endl;
			} else {
				std::cout << loc.redirect.code << " " << loc.redirect.target << std::endl;
			}

			std::cout << "  CGI Extension: " << loc.cgi_extension << std::endl
					  << "  CGI Path: " << loc.cgi_path << std::endl;
		}
	}
}
