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

	// --- Constructors ---
	Response()
		: status_code(200), status_message("OK"), body("<html><body><h1>It works!</h1></body></html>") {
		headers["Content-Type"] = "text/html";

		std::stringstream ss;
		ss << body.size();
		headers["Content-Length"] = ss.str();
	}

	// --- Core Functions ---
	std::string toString() const { return NULL; } // Build full HTTP response
	void setStatus(int code, const std::string &message) { (void)code; (void)message; }
	void setBody(const std::string &content, const std::string &contentType = "text/html") {
		(void)content, (void)contentType; }
};

#endif // RESPONSE_HPP
