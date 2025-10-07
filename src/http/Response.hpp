#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <sstream>  

class Response {
public:
	// --- Response Data ---
	int status_code;
	std::string status_message;
	std::map<std::string, std::string> headers;
	std::string body;
	std::string http_version;
	std::vector<std::string> cookies; // Store Set-Cookie headers separately

	// --- Constructors ---
	Response();


	// --- Core Functions ---
	std::string toString() const;// Build full HTTP response
	void setStatus(int code, const std::string &message);
	void setBody(const std::string &content, const std::string &contentType);
	
	// --- Cookie Functions ---
	void addCookie(const std::string &cookieStr); // Add raw cookie string from CGI
	void setCookie(const std::string &name, const std::string &value, 
		const std::string &path = "/", int maxAge = -1, 
		bool httpOnly = false, bool secure = false,
		const std::string &sameSite = "");
};

#endif // RESPONSE_HPP

