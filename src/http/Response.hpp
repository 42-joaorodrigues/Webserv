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
	std::vector<std::string> cookies;

	// --- Constructors ---
	Response();


	// --- Core Functions ---
	std::string toString() const;// Build full HTTP response
	void setStatus(int code, const std::string &message);
	void setBody(const std::string &content, const std::string &contentType);
	void addCookie(const std::string &cookieLine);
	void setCookie(const std::string &name, const std::string &value, 
	               const std::string &path = "/", int maxAge = 3600, 
	               bool httpOnly = true, bool secure = false, 
	               const std::string &sameSite = "Strict");
	void setSessionCookie(const std::string &name = "session");
};

#endif // RESPONSE_HPP

