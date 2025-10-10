#include "CGIHandler.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <poll.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

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
		dup2(out_pipe[1], STDERR_FILENO); // redirect stderr to stdout to capture all output

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

	// Set pipes to non-blocking
	int flags = fcntl(in_pipe[1], F_GETFL, 0);
	fcntl(in_pipe[1], F_SETFL, flags | O_NONBLOCK);
	flags = fcntl(out_pipe[0], F_GETFL, 0);
	fcntl(out_pipe[0], F_SETFL, flags | O_NONBLOCK);

	size_t total_written = 0;
	std::string output;
	bool write_done = (_body.empty());
	bool read_done = false;

	while (!write_done || !read_done) {
		struct pollfd fds[2];
		int nfds = 0;
		
		// Poll for write if we still have data to send
		if (!write_done) {
			fds[nfds].fd = in_pipe[1];
			fds[nfds].events = POLLOUT;
			nfds++;
		}
		
		// Always poll for read from CGI
		if (!read_done) {
			fds[nfds].fd = out_pipe[0];
			fds[nfds].events = POLLIN;
			nfds++;
		}
		
		if (nfds == 0) break; // Both operations complete
		
		int poll_result = poll(fds, nfds, 5000); // 5 second timeout
		if (poll_result < 0) {
			close(in_pipe[1]);
			close(out_pipe[0]);
			throw std::runtime_error("poll failed");
		}
		if (poll_result == 0) {
			// Timeout - kill CGI and break
			kill(pid, SIGKILL);
			break;
		}
		
		// Handle writing to CGI stdin
		if (!write_done && (fds[0].revents & POLLOUT)) {
			size_t remaining = _body.size() - total_written;
			size_t to_write = (remaining < 8192) ? remaining : 8192;
			ssize_t written = write(in_pipe[1], _body.data() + total_written, to_write);
			if (written > 0) {
				total_written += written;
				if (total_written >= _body.size()) {
					close(in_pipe[1]);
					write_done = true;
				}
			} else if (written < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
				close(in_pipe[1]);
				close(out_pipe[0]);
				throw std::runtime_error("write to CGI failed");
			}
		}
		
		// Handle reading from CGI stdout
		int read_fd_idx = write_done ? 0 : 1;
		if (!read_done && nfds > read_fd_idx && (fds[read_fd_idx].revents & POLLIN)) {
			char buffer[4096];
			ssize_t bytes_read = read(out_pipe[0], buffer, sizeof(buffer));
			if (bytes_read > 0) {
				output.append(buffer, bytes_read);
			} else if (bytes_read == 0) {
				// CGI closed stdout - we're done reading
				read_done = true;
			} else if (errno != EAGAIN && errno != EWOULDBLOCK) {
				close(in_pipe[1]);
				close(out_pipe[0]);
				throw std::runtime_error("read from CGI failed");
			}
		}
		
		// Check for errors or hangup
		for (int i = 0; i < nfds; i++) {
			if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
				if (fds[i].fd == in_pipe[1]) {
					write_done = true;
				} else if (fds[i].fd == out_pipe[0]) {
					read_done = true;
				}
			}
		}
	}

	if (!write_done) {
		close(in_pipe[1]);
	}
	if (!read_done) {
		close(out_pipe[0]);
	}

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


