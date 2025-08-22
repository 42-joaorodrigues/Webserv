#include "config.hpp"

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
		throw std::runtime_error("Expected error page path after codes at line " + peek()._line);
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
