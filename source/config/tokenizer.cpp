#include "tokenizer.hpp"
#include <iostream>
#include <istream>

void tokenizer::advancePosition(char c) {
	if (c == '\n') {
		_line++;
		_col = 1;
	}
	else
		_col++;
}

void tokenizer::tryAddToken(std::string& value, int start_col) {
	if (!value.empty()) {
		_tokens.push_back(Token(value, _line, start_col));
		value.clear();
	}
}

tokenizer::tokenizer(std::istream& input) : _line(1), _col(1) {
	char c;
	std::string current;
	int token_start_col = 1;

	while (input.get(c)) {
		// if space
		if (isspace(static_cast<unsigned char>(c))) {
			tryAddToken(current, token_start_col);
			advancePosition(c);
			continue;
		}

		// handle comments '#'
		if (c == '#') {
			tryAddToken(current, token_start_col);
			while (input.get(c) && c != '\n')
				advancePosition(c);
			advancePosition(c);
			continue;
		}

		// handle symbols '{', '{', ';'
		if (c == '{' || c == '}' || c == ';') {
			tryAddToken(current, token_start_col);
			_tokens.push_back(Token(std::string(1, c), _line, _col));
			advancePosition(c);
			continue;
		}

		// start new token
		if (current.empty())
			token_start_col = _col;

		current += c;
		advancePosition(c);
	}

	// push last token
	tryAddToken(current, token_start_col);
}

void tokenizer::printTokens() const {
	for (std::vector<Token>::const_iterator it = _tokens.begin(); it != _tokens.end(); ++it) {
		std::cout << "\"" << it->_value << "\" [" << it->_line << ":" << it->_col << "]" << std::endl;
	}
}
