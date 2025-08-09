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

class Config
{
	private:
		
	
	public:
	
		vector_servers getServers() const
		{
			vector_servers servers;
			servers.push_back(Server("127.0.0.1", 8080));
			return servers;
		}
		
    
};

#endif