#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include <sstream>

class Response {
public:
	// --- Response Data ---
	int status_code;
	std::string status_message;
	std::map<std::string, std::string> headers;
	std::string body;
	std::string http_version;

	// --- Constructors ---
	Response();


	// --- Core Functions ---
	std::string toString() const;// Build full HTTP response
	void setStatus(int code, const std::string &message);
	void setBody(const std::string &content, const std::string &contentType);
};

#endif // RESPONSE_HPP
