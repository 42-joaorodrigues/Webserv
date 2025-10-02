#ifndef HTTPUTILS_HPP
#define HTTPUTILS_HPP

#include <string>

// Forward declarations
class Socket;

class HttpUtils {
public:
    static std::string getRedirectMessage(int code);
    static std::string getContentType(const std::string& filepath);
    static std::string getErrorPage(int errorcode, int serverid, Socket& socket);
    static std::string getDefaultErrorPage(int status);
};

#endif // HTTPUTILS_HPP