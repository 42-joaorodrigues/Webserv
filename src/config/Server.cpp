/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: joao-alm <joao-alm@student.42luxembourg    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 15:59:01 by naiqing           #+#    #+#             */
/*   Updated: 2025/09/28 16:06:11 by joao-alm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <iostream>

void Server::printServer() const {
	// listen
	std::cout << "Ip: " << _ip << std::endl
			  << "Port: " << _port << std::endl;

	// server name
	std::cout << "Server Name: ";
	for (size_t i = 0; i < _server_names.size(); i++) {
		if (i > 0) std::cout << ", ";
		std::cout << _server_names[i];
	}
	std::cout << std::endl;

	// max body size
	if (_client_max_body_size != -1) {
		std::cout << "Client Max Body Size: " << _client_max_body_size << " bytes" << std::endl;
	}

	// error pages
	if (!_error_pages.empty()) {
		std::cout << "Error Pages:" << std::endl;
		for (std::map<int, std::string>::const_iterator it = _error_pages.begin();
			 it != _error_pages.end(); ++it) {
			std::cout << "  Error " << it->first << " -> " << it->second << std::endl;
			 }
	}
	
	// root
	std::cout << "Root: " << _root << std::endl;
	
	// index
	if (!_indexes.empty()) {
		std::cout << "Index: ";
		for (size_t i = 0; i < _indexes.size(); i++) {
			if (i > 0) std::cout << ", ";
			std::cout << _indexes[i];
		}
		std::cout << std::endl;
	}

	// location
	if (!_locations.empty()) {
		for (std::map<std::string, LocationData>::const_iterator it = _locations.begin();
			 it != _locations.end(); ++it) {
			
			std::cout << " Location " << it->first << ":" << std::endl;

			const LocationData& loc_data = it->second;
			
			// redirect
			if (!loc_data.redirect.empty()) {
				std::map<int, std::string>::const_iterator it = loc_data.redirect.begin();
				std::cout << "  Redirect: " << it->first << " " << it->second << std::endl;
				continue ;
			}
			
			// methods
			if (!loc_data.methods.empty()) {
				std::cout << "  Methods: ";
				for (size_t j = 0; j < loc_data.methods.size(); j++) {
					if (j > 0) std::cout << ", ";
					std::cout << loc_data.methods[j];
				}
				std::cout << std::endl;
			}
			
			// root
			if (!loc_data.root.empty()) {
				std::cout << "  Root: " << loc_data.root << std::endl;
			}

			// index
			if (!loc_data._indexes.empty()) {
				std::cout << "  Index: ";
				for (size_t i = 0; i < loc_data._indexes.size(); i++) {
					if (i > 0) std::cout << ", ";
					std::cout << loc_data._indexes[i];
				}
				std::cout << std::endl;
			}
			
			// autoindex
			if (loc_data.autoindex) {
				std::cout << "  Auto Index: on" << std::endl;
			}
			
			// upload_store
			if (!loc_data.upload_store.empty()) {
				std::cout << "  Upload Store: " << loc_data.upload_store << std::endl;
			}
			
			// cgi_pass
			if (!loc_data.cgi_pass.empty()) {
				std::cout << "  CGI Path: " << loc_data.cgi_pass << std::endl;
			}
			
			// cgi_extension
			if (!loc_data.cgi_extension.empty()) {
				std::cout << "  CGI Extension: " << loc_data.cgi_extension << std::endl;
			}
		}
	}
}
