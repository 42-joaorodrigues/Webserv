#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <map>
#include <string>
#include <vector>

class CGIHandler {
    std::string _interpreter;
    std::string _script_path;
    std::map<std::string, std::string> _env;
	std::string _body;

public:
	CGIHandler(const std::string& interpreter,
			   const std::string& script_path,
			   const std::map<std::string, std::string>& env,
			   const std::string& body);

	std::string execute(); // Run CGI

private:
	std::vector<char *> buildArgv() const;
	std::vector<char *> buildEnvp() const;

	// array helpers
	static char* strDup(const std::string& str);
	static void freeVec(std::vector<char *> vec);
};

#endif // CGIHANDLER_HPP
