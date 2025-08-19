#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <string>
#include <vector>

struct Token {
    std::string _value;
    int         _line;
    int         _col;

    Token(std::string value, int line, int col)
        : _value(value), _line(line), _col(col) {}
};

class Tokenizer {
    std::vector<Token> _tokens;
    int _line;
    int _col;

    void advancePosition(char c);
    void tryAddToken(std::string& current, int start_col);
public:
    Tokenizer(std::istream& input);

    const std::vector<Token>& getTokens() const { return _tokens; }
};

#endif // TOKENIZER_HPP
