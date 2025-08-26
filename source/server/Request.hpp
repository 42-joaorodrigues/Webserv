#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <sstream>

class Request {
public:
	// --- Parsed Data ---
	std::string method;
	std::string uri;
	std::string http_version;
	std::map<std::string, std::string> headers;
	std::string body;

	// --- Constructors ---
	Request()
		: method("GET"), uri("/"), http_version("HTTP/1.1"), body("") {}

	// --- Core Functions ---
	bool parse(const std::string &raw_request) { (void)raw_request; }// Parse raw HTTP request
	bool isValid() const {} // Basic validation
	void clear() {} // Reset for reuse
};

#endif // REQUEST_HPP
