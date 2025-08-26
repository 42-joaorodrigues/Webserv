#include "config.hpp"
#include "../server/server.hpp"

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
