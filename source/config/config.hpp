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

#include "tokenizer.hpp"
#include "../../include/webserv.hpp"

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

	void parseLocation(Server& server);
};

#endif