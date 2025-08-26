#include "CGIHandler.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

CGIHandler::CGIHandler(const std::string& interpreter,
						const std::string& script_path,
						const std::map<std::string, std::string>& env,
						const std::string& body)
	: _interpreter(interpreter), _script_path(script_path), _env(env), _body(body) {}

std::string CGIHandler::execute() {
	int in_pipe[2];
	int out_pipe[2];

	if (pipe(in_pipe) == -1 || pipe(out_pipe) == -1) {
		throw std::runtime_error("failed to create pipes");
	}

	pid_t pid = fork();
	if (pid < 0) {
		throw std::runtime_error("fork failed");
	}
	if (pid == 0) { // child
		dup2(in_pipe[0], STDIN_FILENO); // read end of in_pipe redirect
		dup2(out_pipe[1], STDOUT_FILENO); // write end of out_pipe redirect

		close(in_pipe[0]); close(in_pipe[1]);
		close(out_pipe[0]); close(out_pipe[1]);

		std::vector<char*> argv = buildArgv();
		std::vector<char*> envp = buildEnvp();

		execve(argv[0], argv.data(), envp.data());

		perror("execve failed");
		freeVec(argv);
		freeVec(envp);
		exit(1);
	}
	// parent
	close(in_pipe[0]); // read end of in_pipe not used in parent
	close(out_pipe[1]); // write end of out_pipe not used in parent

	// write body
	if (!_body.empty()) {
		ssize_t written = write(in_pipe[1], _body.c_str(), _body.size());
		if (written == -1) {
			close(in_pipe[1]);
			close(out_pipe[0]);
			throw std::runtime_error("failed to write to CGI");
		}
	}
	close(in_pipe[1]);

	// read from CGI
	std::string output;
	char buffer[4096];
	ssize_t n;
	while ((n = read(out_pipe[0], buffer, sizeof(buffer))) > 0) {
		output.append(buffer, n);
	}
	close(out_pipe[0]);

	// wait for child
	int status;
	waitpid(pid, &status, 0);

	return output;
}

std::vector<char*> CGIHandler::buildArgv() const {
	std::vector<char *> argv;

	try {
		argv.push_back(strDup(_interpreter));
		argv.push_back(strDup(_script_path));
		argv.push_back(NULL);
	} catch (...) {
		freeVec(argv);
		throw;
	}
	return argv;
}

std::vector<char*> CGIHandler::buildEnvp() const {
	std::vector<char *> envp;

	try {
		for (std::map<std::string, std::string>::const_iterator it = _env.begin();
			it != _env.end(); ++it) {
			std::string entry = it->first + "=" + it->second;
			envp.push_back(strDup(entry));
		}
		envp.push_back(NULL);
	} catch (...) {
		freeVec(envp);
		throw;
	}
	return envp;
}

char* CGIHandler::strDup(const std::string& str) {
	char *p = new char[str.size() + 1];
	std::memcpy(p, str.c_str(), str.size() + 1);
	return p;
}

void CGIHandler::freeVec(std::vector<char*> vec) {
	for (size_t i = 0; i < vec.size(); i++) {
		if (vec[i]) delete[] vec[i];
	}
	vec.clear();
}


