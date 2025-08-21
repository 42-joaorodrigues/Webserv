#include "ServerParser.hpp"

ServerParser::ServerParser(const std::vector<Token>& tokens, size_t& pos)
	: _tokens(tokens), _pos(pos) {

	advance();
	expect("{");

	while (!match("}")) {
		std::string key = peek()._value;
		advance();

		if (key == "listen") parseListen();
		else if (key == "server_name") parseServerName();
		else if (key == "root") parseRoot();
		else if (key == "index") parseIndex();
		else if (key == "error_page") parseErrorPage();
		else if (key == "client_max_body_size") parseBodySize();
		else if (key == "location") parseLocation();

		else {
			std::ostringstream oss;
			oss << "Unknown directive '" << key << "' at line " << peek()._line;
			throw std::runtime_error(oss.str());
		}
	}
}

void ServerParser::parseListen() {
	std::string input = peek()._value;

	// look for ':' separator
	std::string::size_type pos = input.find(":");
	if (pos == std::string::npos) {
		throw std::runtime_error("Invalid host:port format: " + input);
	}

	// set ip
	_server.setIp(input.substr(0, pos));

	// set port
	std::string port_str = input.substr(pos + 1);
	int port = std::atoi(port_str.c_str());
	if (port <= 0 || port > 65535) {
		throw std::runtime_error("Invalid port number: " + port_str);
	}
	_server.setPort(port);

	advance();
	expect(";");
}

void ServerParser::parseServerName() {
	while (!match(";")) {
		_server.addServerName(peek()._value);
		advance();
	}
}

void ServerParser::parseRoot() {
	_server.setRoot(peek()._value);
	advance();
	expect(";");
}

void ServerParser::parseIndex() {
	while (!match(";")) {
		_server.addIndex(peek()._value);
		advance();
	}
}

void ServerParser::parseErrorPage() {
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
		_server.addErrorPage(codes[i], path);
	}
}

void ServerParser::parseBodySize() {
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

	_server.setClientMaxBodySize(static_cast<size_t>(value) * multiplier);
	advance();
	expect(";");
}

void ServerParser::parseLocation() {
	Location location;

	location.path = peek()._value;
	advance();
	expect("{");

	// temporary
	while (!match("}")) {
		std::string key = peek()._value;
		advance();

	}
}

bool ServerParser::match(const std::string& value) {
	if (_pos < _tokens.size() && _tokens[_pos]._value == value) {
		advance();
		return true;
	}
	return false;
}

const Token& ServerParser::peek() const {
	if (_pos >= _tokens.size()) {
		throw std::runtime_error("Unexpected end of tokens");
	}
	return _tokens[_pos];
}

void ServerParser::expect(const std::string& value) {
	if (!match(value)) {
		std::ostringstream oss;
		oss << "Expected '" << value << "' at line " << peek()._line << " col " << peek()._col;
		throw std::runtime_error(oss.str());
	}
}

void ServerParser::advance() {
	_pos++;
}

