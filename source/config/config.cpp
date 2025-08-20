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

Config::Config(const std::string& filename) {
	std::ifstream file(filename.c_str());
	if (!file)
		throw std::runtime_error("Couldn't open config file.");

	Tokenizer tokens(file);
	_tokens = tokens.getTokens();
	_pos = 0;

	while (_pos < _tokens.size()) {
		if (match("server")) {
			parseServer();
		} else {
			std::ostringstream oss;
			oss << "Unexpected '" << peek()._value << "' at line " << peek()._line;
			throw std::runtime_error(oss.str());
		}
	}
}

void Config::parseServer() {
	Server server;
	expect("{");

	while (!match("}")) {
		std::string key = peek()._value;
		advance();

		if (key == "listen") {
			parseListen(server);
		}
		else if (key == "server_name") {
			server.setServerName(peek()._value);
			advance();
			expect(";");
		}
		else if (key == "root") {
			server.setRoot(peek()._value);
			advance();
			expect(";");
		}
		else if (key == "index") {
			server.setIndex(peek()._value);
			advance();
			expect(";");
		}
		else if (key == "error_page") {
			int code = std::atoi(peek()._value.c_str());
			advance();
			std::string path = peek()._value;
			advance();
			server.addErrorPage(code, path);
			advance();
			expect(";");
		}
		else if (key == "location") {
			parseLocation(server);
		}
		else {
			std::ostringstream oss;
			oss << "Unknown directive '" << key << "' at line " << peek()._line;
			throw std::runtime_error(oss.str());
		}
	}

	_servers.push_back(server);
}

void Config::parseListen(Server& server) {
	std::string input = peek()._value;

	// look for ':' separator
	std::string::size_type pos = input.find(":");
	if (pos == std::string::npos) {
		throw std::runtime_error("Invalid host:port format: " + input);
	}

	// set ip
	server.setIp(input.substr(0, pos));

	// set port
	std::string port_str = input.substr(pos + 1);
	int port = std::atoi(port_str.c_str());
	if (port <= 0 || port > 65535) {
		throw std::runtime_error("Invalid port number: " + port_str);
	}
	server.setPort(port);

	advance();
	expect(";");
}

void Config::parseLocation(Server& server) {
	(void)server;
	Location location;

	location.path = peek()._value;
	expect("{");

	while (!match("}")) {
		std::string key = peek()._value;
		advance();

	}
}

bool Config::match(const std::string& value) {
	if (_pos < _tokens.size() && _tokens[_pos]._value == value) {
		advance();
		return true;
	}
	return false;
}

const Token& Config::peek() const {
	if (_pos >= _tokens.size()) {
		throw std::runtime_error("Unexpected end of tokens");
	}
	return _tokens[_pos];
}

void Config::expect(const std::string& value) {
	if (!match(value)) {
		std::ostringstream oss;
		oss << "Expected '" << value << "' at line " << peek()._line << " col " << peek()._col;
		throw std::runtime_error(oss.str());
	}
}

void Config::advance() {
	_pos++;
}
