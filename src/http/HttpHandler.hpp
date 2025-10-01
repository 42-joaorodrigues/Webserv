#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include <string>

// Forward declarations
class Request;
class Response;
class Socket;
struct MatchedLocation;

class HttpHandler {
public:
    static void handleLocationRequest(const Request& req, const MatchedLocation& matched, int serverid, Socket& socket, Response& response);
    static int handleHttpRequest(int client_fd, Socket& socket);
    static std::string readFullHttpRequest(int client_fd);
};

#endif // HTTPHANDLER_HPP