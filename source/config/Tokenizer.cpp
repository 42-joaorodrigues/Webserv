#include "Tokenizer.hpp"
#include <istream>

void Tokenizer::advancePosition(char c) {
	if (c == '\n') {
		_line++;
		_col = 1;
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

Tokenizer::Tokenizer(std::istream& input) : _line(1), _col(1) {
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


	}
}
