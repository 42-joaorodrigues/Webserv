#ifndef ERRORPAGE_HPP
#define ERRORPAGE_HPP

#include <string>
#include <sstream>
#include <map>

class ErrorPage {
public:
    // Static generator (default template)
    static std::string generate(int code, const std::string &message) {
        std::ostringstream oss;
        oss << "<html><head><title>" << code << " " << message << "</title></head>"
            << "<body><h1>" << code << " " << message << "</h1>"
            << "<p>The server encountered an error.</p>"
            << "<hr><em>webserv</em></body></html>";
        return oss.str();
    }

    // Predefined messages (can be expanded)
    static std::string defaultMessage(int code) {
        static std::map<int, std::string> messages = {
            {400, "Bad Request"},
            {403, "Forbidden"},
            {404, "Not Found"},
            {405, "Method Not Allowed"},
            {413, "Payload Too Large"},
            {500, "Internal Server Error"},
            {502, "Bad Gateway"},
            {504, "Gateway Timeout"}
        };
        return (messages.count(code) ? messages[code] : "Unknown Error");
    }
};

#endif // ERRORPAGE_HPP

