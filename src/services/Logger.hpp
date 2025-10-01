#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <iomanip>

class Logger {
public:
    enum LogLevel {
        DEBUG,
        INFO,
        WARN,
        LOG_ERROR
    };

    // Main logging methods
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);
    
    // Specialized logging for server events
    static void serverStart(const std::string& message);
    static void serverStop(const std::string& message);
    static void connection(const std::string& message);
    static void request(const std::string& method, const std::string& uri, int socket_fd);
    static void response(int status_code, int socket_fd);
    static void timeout(int socket_fd);

private:
    // Color constants
    static const std::string RESET;
    static const std::string BOLD;
    static const std::string DIM;
    
    // Colors (matching your Makefile style)
    static const std::string CYAN;      // Info messages
    static const std::string YELLOW;    // Warnings/Debug
    static const std::string GREEN;     // Success/Responses
    static const std::string PINK;      // Errors/Timeouts
    static const std::string BLUE;      // Connections
    static const std::string WHITE;     // General text
    
    // Helper methods
    static std::string getCurrentTimestamp();
    static std::string formatLevel(LogLevel level);
    static std::string getLevelColor(LogLevel level);
    static void log(LogLevel level, const std::string& message);
};

#endif // LOGGER_HPP