#ifndef SERVER_HPP
#define SERVER_HPP

#include "../../include/webserv.hpp"

struct Location {
	std::string path;
	std::vector<std::string> methods;
	std::string upload_store;
	std::string cgi_extension;
	std::string cgi_path;
};

class Server {
	std::string _ip; // IP address for the server
	int _port; // Port number for the server
	std::string _server_name;
	std::string _root;
	std::string _index;
	std::map<int, std::string> _error_pages;
	std::vector<Location> _locations;

public:
	Server() : _port(-1) {}

	// getters
	std::string getIp() const { return _ip; }
	int	getPort() const { return _port; }
	const std::string& getServerName() const { return _server_name; }
	const std::string& getRoot() const { return _root; }
	const std::string& getIndex() const { return _index; }
	const std::map<int, std::string>& getErrorPages() const { return _error_pages; }
	const std::vector<Location>& getLocations() const { return _locations; }

	// setters
	void setIp(const std::string& ip) { _ip = ip; }
	void setPort(int port) { _port = port; }
	void setServerName(const std::string& server_name) { _server_name = server_name; }
	void setRoot(const std::string& root) { _root = root; }
	void setIndex(const std::string& index) { _index = index; }
	void setErrorPages(const std::map<int, std::string>& error_pages) { _error_pages = error_pages; }
	void setLocations(const std::vector<Location>& locations) { _locations = locations; }

	// adders
	void addErrorPage(int code, const std::string& path) { _error_pages[code] = path; }
	void addLocation(const Location& loc) { _locations.push_back(loc); }
};

#endif