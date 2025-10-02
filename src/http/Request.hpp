#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <cstdlib> 
#include <limits>

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
	std::string dechunk(const std::string &chunk);
	bool isValidChunkSizeLine(const std::string &line);
	
};

#endif // REQUEST_HPP
