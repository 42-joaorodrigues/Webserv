/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nacao <nacao@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 14:06:07 by nacao             #+#    #+#             */
/*   Updated: 2025/08/07 14:06:08 by nacao            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config.hpp"

Config::Config(const std::string& filename) {
	std::ifstream file(filename.c_str()); // try to open file
	if (!file.is_open()) // can't open file
		throw std::runtime_error("Could not open config file: " + filename);

	std::string line; // line to read
	int line_num = 0; // number of lines

	while (std::getline(file, line)) { // read loop
		line_num++;

		line = line.substr(line.find_first_not_of(" \t")); // skip starting spaces and tabs
		if (line.empty() || line[0] == '#') // skip empty lines or comments
			continue;

		std::istringstream iss(line);
		std::string key, value;

	}
}
