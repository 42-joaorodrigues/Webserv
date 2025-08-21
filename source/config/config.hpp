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

#include "../../include/webserv.hpp"

class Config {
	std::vector<Server> _servers;

public:
	Config(const std::string& filename);

	// getters
	std::vector<Server> getServers() { return _servers; }

	// debug
	void printConfig() const;
};

#endif