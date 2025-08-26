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