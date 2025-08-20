#include "Tokenizer.hpp"
#include <iostream>
#include <istream>

void Tokenizer::advancePosition(char c) {
	if (c == '\n') {
		_line++;
		_col = 0;
	}
	else
		_col++;
}

void Tokenizer::tryAddToken(std::string& value, int start_col) {
	if (!value.empty()) {
		_tokens.push_back(Token(value, _line, start_col));
		value.clear();
	}
}

Tokenizer::Tokenizer(std::istream& input) : _line(1), _col(0) {
	char c;
	std::string current;
	int token_start_col = 0;

	while (input.get(c)) {
		advancePosition(c);

		// if space
		if (isspace(static_cast<unsigned char>(c))) {
			tryAddToken(current, token_start_col);
			continue;
		}

		// handle comments '#'
		if (c == '#') {
			tryAddToken(current, token_start_col);
			while (input.get(c) && c != '\n')
				advancePosition(c);
			continue;
		}

		// handle symbols '{', '{', ';'
		if (c == '{' || c == '}' || c == ';') {
			tryAddToken(current, token_start_col);
			_tokens.push_back(Token(std::string(1, c), _line, _col));
			continue;
		}

		// start new token
		if (current.empty())
			token_start_col = _col;

		current += c;
	}

	// push last token
	tryAddToken(current, token_start_col);
}

void Tokenizer::printTokens() const {
	for (std::vector<Token>::const_iterator it = _tokens.begin(); it != _tokens.end(); ++it) {
		std::cout << "\"" << it->_value << "\" [" << it->_line << ":" << it->_col << "]" << std::endl;
	}
}
