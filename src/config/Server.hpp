#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <map>
#include <sys/types.h>

// Represents a "location" block inside a server
struct LocationData {
    std::vector<std::string> methods;          // Allowed HTTP methods (GET, POST, DELETE, etc.)
    std::string root;                          // Directory for this route (overrides server root if set)
    std::string alias;                         // Alias path (alternative to root, maps location directly)
    std::vector<std::string> _indexes;                       // Default file (e.g., index.html)
    bool autoindex;                            // Directory listing enabled/disabled
    std::string upload_store;                // Upload directory (if enabled)
    std::map<int, std::string> redirect;                      // Redirect target (e.g., "http://...")
    std::string cgi_pass;                 // Path to CGI executable (e.g., "/usr/bin/php-cgi")
	std::string cgi_extension;                 // File extension that triggers CGI (e.g., ".php")
    ssize_t client_max_body_size;             // Location-specific body size limit (-1 = inherit from server)

    LocationData() : autoindex(false), client_max_body_size(-1) {}
};

// Represents a "server" block
class Server {
    std::string _ip;                           // IP address for the server
    int _port;                                 // Port number
    std::vector<std::string> _server_names;    // Multiple possible server_name values
    std::string _root;                         // Root directory
    std::vector<std::string> _indexes;                        // Default index file
    std::map<int, std::string> _error_pages;   // Error code -> file
    ssize_t _client_max_body_size;              // Limit on request body size
    std::map<std::string, LocationData> _locations;          // All location blocks

public:
    Server() : _port(-1), _client_max_body_size(-1) {}

    // --- Getters ---
    std::string getIp() const { return _ip; }
    int getPort() const { return _port; }
    const std::vector<std::string>& getServerNames() const { return _server_names; }
    const std::string& getRoot() const { return _root; }
    const std::vector<std::string>& getIndexes() const { return _indexes; }
    const std::map<int, std::string>& getErrorPages() const { return _error_pages; }
    ssize_t getClientMaxBodySize() const { return _client_max_body_size; }
    const std::map<std::string, LocationData>& getLocations() const { return _locations; }

    // --- Setters ---
    void setIp(const std::string& ip) { _ip = ip; }
    void setPort(int port) { _port = port; }
    void setServerNames(const std::vector<std::string>& server_names) { _server_names = server_names; }
    void setRoot(const std::string& root) { _root = root; }
    void setIndexes(const std::vector<std::string>& indexes) { _indexes = indexes; }
    void setClientMaxBodySize(ssize_t size) { _client_max_body_size = size; }
    void setErrorPages(const std::map<int, std::string>& error_pages) { _error_pages = error_pages; }
    void setLocations(const std::map<std::string, LocationData>& locations) { _locations = locations; }

    // --- Adders ---
    void addServerName(const std::string& server_name) { _server_names.push_back(server_name); }
    void addIndex(const std::string& index) { _indexes.push_back(index); }
    void addErrorPage(int code, const std::string& path) { _error_pages[code] = path; }
    void addLocation(const std::string path, const LocationData& loc_data) { _locations[path] = loc_data; }

    // debug
    void printServer() const;
};

#endif
