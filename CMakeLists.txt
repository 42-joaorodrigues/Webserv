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
		source/config/Tokenizer.hpp
		source/config/ConfigTokenUtils.cpp
		source/config/ConfigServerParser.cpp
		source/config/ConfigLocationParser.cpp
		source/cgi/CGIHandler.cpp
		source/cgi/CGIHandler.hpp
		source/cgi/main.cpp
		source/server/ErrorPage.hpp
		source/server/Request.hpp
		source/server/Response.hpp)
