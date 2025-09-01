#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <cstdlib> 

class Request {
public:
	// --- Parsed Data ---
	std::string method;
	std::string uri;
	std::string http_version;
	std::map<std::string, std::string> headers;
	std::string body;

	// --- Constructors ---
	Request();

	// --- Core Functions ---
	bool parse(const std::string &raw_request); // Parse raw HTTP request
	bool isValid(); // Basic validation
	void clear(); // Reset for reuse
};

#endif // REQUEST_HPP
