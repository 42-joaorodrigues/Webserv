/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: joao-alm <joao-alm@student.42luxembourg    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 14:06:04 by nacao             #+#    #+#             */
/*   Updated: 2025/10/09 23:37:43 by joao-alm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Tokenizer.hpp"
#include "Server.hpp"

struct LocationData;

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

	void parseLocMethods(LocationData& loc_data);
	void parseLocRoot(LocationData& loc_data);
	void parseLocAlias(LocationData& loc_data);
	void parseLocIndex(LocationData& loc_data);
	void parseLocAutoIndex(LocationData& loc_data);
	void parseLocUpload(LocationData& loc_data);
	void parseLocRedirect(LocationData& loc_data);
	void parseLocCgiPass(LocationData& loc_data);
	void parseLocCgiExt(LocationData& loc_data);
	// ---

	// location inheritance
	
};

#endif