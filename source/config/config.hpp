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

class Config {
	std::vector<Server> _servers;

public:
	Config(const std::string& filename);

	// getters
	std::vector<Server> getServers() { return _servers; }

private:
	// helpers
	std::vector<Token> _tokens;
	size_t _pos;

	void parseServer();
	void parseListen(Server& server);
	void parseLocation(Server& server);

	bool match(const std::string& value);
	const Token& peek() const;
	void expect(const std::string& value);
	void advance();
};

#endif