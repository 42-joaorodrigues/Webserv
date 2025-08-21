#ifndef SERVERPARSER_HPP
#define SERVERPARSER_HPP

#include "Tokenizer.hpp"
#include "../server/server.hpp"

class ServerParser {
    Server _server;

public:
    ServerParser(const std::vector<Token>& tokens, size_t& pos);

    const Server& getServer() const { return _server; }

private:
    // helpers
    const std::vector<Token>& _tokens;
    size_t& _pos;

    // parsers
    void parseListen();
    void parseServerName();
    void parseRoot();
    void parseIndex();
    void parseErrorPage();
    void parseBodySize();
    void parseLocation();

    // vector navigation
    bool match(const std::string& value);
    const Token& peek() const;
    void expect(const std::string& value);
    void advance();
};

#endif // SERVERPARSER_HPP
