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
	vector_servers _servers;
public:
	Config(const std::string& filename);

	vector_servers getServers() { return _servers; }
};

#endif