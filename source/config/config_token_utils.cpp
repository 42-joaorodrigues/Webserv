#include "config.hpp"

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
		std::ostringstream oss;
		oss << "Expected '" << value << "' at line " << peek()._line << " col " << peek()._col;
		throw std::runtime_error(oss.str());
	}
}

void Config::advance() {
	_pos++;
}