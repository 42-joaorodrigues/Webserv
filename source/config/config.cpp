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
#include "Tokenizer.hpp"
#include "ServerParser.hpp"

Config::Config(const std::string& filename) {
	std::ifstream file(filename.c_str());
	if (!file)
		throw std::runtime_error("Couldn't open config file.");

	Tokenizer tokenizer(file);
	// tokens.printTokens();
	const std::vector<Token>& tokens = tokenizer.getTokens();
	size_t pos = 0;

	while (pos < tokens.size()) {
		if (tokens[pos]._value == "server") {
			ServerParser sp(tokens, pos);
			_servers.push_back(sp.getServer());
		} else {
			std::ostringstream oss;
			oss << "Unexpected '" << tokens[pos]._value << "' at line " << tokens[pos]._line;
			throw std::runtime_error(oss.str());
		}
	}
}

void Config::printConfig() const {
	std::cout << std::endl << "Servers" << std::endl;
	if (_servers.empty()) {
		std::cout << "(none)" << std::endl;
	} else {
		for (size_t i = 0; i < _servers.size(); i++) {
			std::cout << std::endl << "Server " << i << ":" << std::endl;
			_servers[i].printServer();
		}
	}
}
