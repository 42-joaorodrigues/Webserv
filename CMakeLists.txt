cmake_minimum_required(VERSION 3.31)
project(Webserv)

set(CMAKE_CXX_STANDARD 98)

add_executable(Webserv source/main.cpp
		Makefile
		source/config/config.cpp
		source/connection/epoll.cpp
		source/connection/socket.cpp
		source/connection/socket.hpp
		source/server/server.cpp
		source/server/server.hpp
		source/config/Tokenizer.cpp
		source/config/Tokenizer.hpp)
