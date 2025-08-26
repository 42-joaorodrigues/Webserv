/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: naiqing <naiqing@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 14:06:04 by nacao             #+#    #+#             */
/*   Updated: 2025/08/09 10:18:27 by naiqing          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Tokenizer.hpp"
#include "../../include/webserv.hpp"

struct Location;

class Config {
	std::vector<Server> _servers;

public:
	Config(const std::string& filename);
	std::vector<Server> getServers() { return _servers; } // getters
	void printConfig() const; // debug

private:
	// token utils
	std::vector<Token> _tokens;
	size_t _pos;

	bool match(const std::string& value);
	const Token& peek() const;
	void expect(const std::string& value);
	void advance();
	// ---

	// server parsers
	void parseServer(Server& server);

	void parseListen(Server& server);
	void parseServerName(Server& server);
	void parseRoot(Server& server);
	void parseIndex(Server& server);
	void parseErrorPage(Server& server);
	void parseBodySize(Server& server);
	// ---

	// location parsers
	void parseLocation(Server& server);

	void parseLocMethods(Location& location);
	void parseLocRoot(Location& location);
	void parseLocIndex(Location& location);
	void parseLocAutoIndex(Location& location);
	void parseLocUpload(Location& location);
	void parseLocRedirect(Location& location);
	void parseLocCgiExt(Location& location);
	void parseLocCgiPath(Location& location);
	// ---
};

#endif