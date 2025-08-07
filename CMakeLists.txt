cmake_minimum_required(VERSION 3.31)
project(Webserv)

set(CMAKE_CXX_STANDARD 98)

add_executable(Webserv source/main.cpp
		Makefile
		source/config/config.cpp
		source/epoll/epoll.cpp
		source/epoll/epoll.hpp
		source/server/server.cpp
		source/socket/socket.cpp)
