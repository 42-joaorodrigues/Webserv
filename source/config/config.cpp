/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nacao <nacao@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 14:06:07 by nacao             #+#    #+#             */
/*   Updated: 2025/08/07 14:06:08 by nacao            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config.hpp"
#include "tokenizer.hpp"

Config::Config(const std::string& filename) {
	std::ifstream file(filename.c_str());
	if (!file)
		throw std::runtime_error("Couldn't open config file.");

	tokenizer tokenizer(file);
	// tokenizer.printTokens();

	_tokens = tokenizer.getTokens();
	_pos = 0;

	while (_pos < _tokens.size()) {
		if (match("server")) {
			Server server;
			parseServer(server);

			_servers.push_back(server);
		} else {
			std::ostringstream oss;
			oss << "Unexpected '" << peek()._value << "' at line " << peek()._line;
			throw std::runtime_error(oss.str());
		}
	}
}

void Config::printConfig() const {
	std::cout << std::endl << "Servers:" << std::endl;
	if (_servers.empty()) {
		std::cout << "(none)" << std::endl;
	} else {
		for (size_t i = 0; i < _servers.size(); i++) {
			std::cout << std::endl << "Server " << i << ":" << std::endl;
			_servers[i].printServer();
		}
	}
}
