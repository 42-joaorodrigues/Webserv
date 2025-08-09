#ifndef TYPE_HPP
#define TYPE_HPP

#include <vector>
#include <map>
#include <string>

class Socket;
class Server;
class Config;

typedef std::vector<int> vector_int; // Vector of integers, used for file descriptors
typedef std::vector<Server> vector_servers; // Vector of Server objects, used for managing multiple servers
typedef std::map<int, int> mapSocket; // Map to associate file descriptors with their corresponding server or socket

#endif