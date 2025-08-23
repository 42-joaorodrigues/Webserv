#include "cgi_handler.hpp"
#include "iostream"

int main() {
	std::map<std::string, std::string> env;
	env["REQUEST_METHOD"] = "GET";
	env["CONTENT_LENGTH"] = "3";

	std::string body = "Hello World";

	CGIHandler cgi("/usr/bin/python3", "cgi_test.py", env, body);

	try {
		std::string output = cgi.execute();
		std::cout << "\nCGI:\n" << output << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}