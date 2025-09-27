/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: joao-alm <joao-alm@student.42luxembourg    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 14:06:07 by nacao             #+#    #+#             */
/*   Updated: 2025/09/27 20:12:57 by joao-alm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>

Config::Config(const std::string& filename) {
	std::ifstream file(filename.c_str());
	if (!file)
		throw std::runtime_error("Couldn't open config file.");

	Tokenizer tokenizer(file);
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

void Config::parseServer(Server& server) {
	advance();
	expect("{");
	advance();

	while (!match("}")) {
		Token key_tkn = peek();
		std::string key = key_tkn._value;
		advance();

		if (key == "listen") parseListen(server);
		else if (key == "server_name") parseServerName(server);
		else if (key == "root") parseRoot(server);
		else if (key == "index") parseIndex(server);
		else if (key == "error_page") parseErrorPage(server);
		else if (key == "client_max_body_size") parseBodySize(server);
		else if (key == "location") parseLocation(server);
		else {
			std::ostringstream oss;
			oss << "Unknown directive '" << key << "' at line " << key_tkn._line;
			throw std::runtime_error(oss.str());
		}

		advance();
	}
	advance();
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

void Config::parseServerName(Server& server) {
	while (!match(";")) {
		server.addServerName(peek()._value);
		advance();
	}
}

void Config::parseRoot(Server& server) {
	server.setRoot(peek()._value);
	advance();
	expect(";");
}

void Config::parseIndex(Server& server) {
	while (!match(";")) {
		server.addIndex(peek()._value);
		advance();
	}
}

void Config::parseErrorPage(Server& server) {
	std::vector<int> codes;

	// collect codes until we hit something that looks like a path (starts with "/" or a non-digit)
	while (_pos < _tokens.size() && std::isdigit(peek()._value[0])) {
		int code = std::atoi(peek()._value.c_str());
		codes.push_back(code);
		advance();
	}

	// now the path
	if (_pos >= _tokens.size()) {
		std::ostringstream oss;
		oss << "Expected error page path after codes at line " << peek()._line;
		throw std::runtime_error(oss.str());
	}
	std::string path = peek()._value;
	advance();
	expect(";");

	// assign path to all codes
	for (size_t i = 0; i < codes.size(); i++) {
		server.addErrorPage(codes[i], path);
	}
}

void Config::parseBodySize(Server& server) {
	const std::string str = peek()._value;
	if (str.empty())
		throw std::invalid_argument("Empty body size");

	// Separate number and suffix
	size_t i = 0;
	while (i < str.size() && isdigit(str[i])) {
		++i;
	}

	if (i == 0)
		throw std::invalid_argument("No numeric part in body size");

	// numeric part
	long value = std::strtol(str.substr(0, i).c_str(), NULL, 10);
	if (value < 0)
		throw std::invalid_argument("Negative body size not allowed");

	// suffix part (could be empty = bytes)
	size_t multiplier = 1;
	if (i < str.size()) {
		char suffix = str[i];
		switch (suffix) {
		case 'K': case 'k': multiplier = 1024; break;
		case 'M': case 'm': multiplier = 1024 * 1024; break;
		case 'G': case 'g': multiplier = 1024 * 1024 * 1024; break;
		default:
			throw std::invalid_argument("Unknown size suffix");
		}
	}

	server.setClientMaxBodySize(static_cast<size_t>(value) * multiplier);
	advance();
	expect(";");
}

void Config::parseLocation(Server& server) {
	Location location;

	location.path = peek()._value;
	advance();
	expect("{");
	advance();

	while (!match("}")) {
		Token key_tkn = peek();
		std::string key = key_tkn._value;
		advance();

		if (key == "allow_methods") parseLocMethods(location);
		else if (key == "root") parseLocRoot(location);
		else if (key == "index") parseLocIndex(location);
		else if (key == "autoindex") parseLocAutoIndex(location);
		else if (key == "upload_store") parseLocUpload(location);
		else if (key == "return") parseLocRedirect(location);
		else if (key == "cgi_extension") parseLocCgiExt(location);
		else if (key == "cgi_pass") parseLocCgiPath(location);
		else {
			std::ostringstream oss;
			oss << "Unknown directive '" << key << "' at line " << key_tkn._line;
			throw std::runtime_error(oss.str());
		}
		advance();
	}

	server.addLocation(location);
}

void Config::parseLocMethods(Location& location) {
	while (!match(";")) {
		location.methods.push_back(peek()._value);
		advance();
	}
}

void Config::parseLocRoot(Location& location) {
	location.root = peek()._value;
	advance();
	expect(";");
}

void Config::parseLocIndex(Location& location) {
	location.index = peek()._value;
	advance();
	expect(";");
}

void Config::parseLocAutoIndex(Location& location) {
	std::string value;
	value = peek()._value;

	if (value != "on" && value != "off") {
		std::ostringstream oss;
		oss << "Invalid value for 'autoindex' at line '" << peek()._line
			<< "'. Valid options: <on/off> (case sensitive)" << std::endl;
		throw std::runtime_error(oss.str());
	}
	location.autoindex = value == "on";

	advance();
	expect(";");
}

void Config::parseLocUpload(Location& location) {
	location.upload_store = peek()._value;
	advance();
	expect(";");
}

void Config::parseLocRedirect(Location& location) {
	location.redirect.exists = true;

	// code
	std::string value = peek()._value;
	for (size_t i = 0; i < value.length(); i++) {
		if (!isdigit(value[i])) {
			std::ostringstream oss;
			oss << "Invalid Redirect code at line '" << peek()._line << "'" << std::endl;
			throw std::runtime_error(oss.str());
		}
	}
	location.redirect.code = std::atoi(value.c_str());
	advance();

	location.redirect.target = peek()._value;
	advance();
	expect(";");
}

void Config::parseLocCgiExt(Location& location) {
	location.cgi_extension = peek()._value;
	advance();
	expect(";");
}

void Config::parseLocCgiPath(Location& location) {
	location.cgi_path = peek()._value;
	advance();
	expect(";");
}


bool Config::match(const std::string& value) {
	if (_pos < _tokens.size() && _tokens[_pos]._value == value) {
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
		size_t old_pos = _pos > 0 ? _pos - 1 : _pos;
		Token old_token = _tokens[old_pos];

		std::ostringstream oss;
		oss << "Expected '" << value << "' at line " << old_token._line
			<< " col " << old_token._col + old_token._value.length();
		throw std::runtime_error(oss.str());
	}
}

void Config::advance() {
	_pos++;
}
