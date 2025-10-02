#include "Logger.hpp"
#include <cstring>

// Color constants
const std::string Logger::RESET = "\033[0m";
const std::string Logger::BOLD = "\033[1m";
const std::string Logger::DIM = "\033[2m";

// Colors simplified to cyan and blue only
const std::string Logger::CYAN = "\033[38;2;100;200;255m";      // Requests/Connections (cyan)
const std::string Logger::YELLOW = "\033[38;2;255;231;151m";    // Warnings/Debug (yellow)
const std::string Logger::GREEN = "\033[38;2;80;120;200m";      // Server events/Responses (blue)
const std::string Logger::PINK = "\033[38;2;211;125;174m";      // Errors/Timeouts (pink)
const std::string Logger::BLUE = "\033[38;2;80;120;200m";       // Same as GREEN (blue)
const std::string Logger::WHITE = "\033[37m";                   // General text

std::string Logger::getCurrentTimestamp() {
    std::time_t now = std::time(0);
    std::tm* timeinfo = std::localtime(&now);
    
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return std::string(buffer);
}

std::string Logger::formatLevel(LogLevel level) {
    switch (level) {
        case DEBUG: return "DEBUG";
        case INFO:  return "INFO ";
        case WARN:  return "WARN ";
        case LOG_ERROR: return "ERROR";
        default:    return "UNKN ";
    }
}

std::string Logger::getLevelColor(LogLevel level) {
    switch (level) {
        case DEBUG: return YELLOW;
        case INFO:  return CYAN;
        case WARN:  return YELLOW;
        case LOG_ERROR: return PINK;
        default:    return WHITE;
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    std::string timestamp = getCurrentTimestamp();
    std::string levelStr = formatLevel(level);
    std::string color = getLevelColor(level);
    
    std::cout << DIM << "[" << timestamp << "]" << RESET 
              << "  " << color << "[" << levelStr << "]" << RESET 
              << "  " << color << message << RESET << std::endl;
}

void Logger::debug(const std::string& message) {
    log(DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(INFO, message);
}

void Logger::warn(const std::string& message) {
    log(WARN, message);
}

void Logger::error(const std::string& message) {
    log(LOG_ERROR, message);
}

void Logger::serverStart(const std::string& message) {
    std::string timestamp = getCurrentTimestamp();
    std::cout << DIM << "[" << timestamp << "]" << RESET 
              << "  " << GREEN << "[INFO ]" << RESET 
              << "  " << GREEN << message << RESET << std::endl;
}

void Logger::serverStop(const std::string& message) {
    std::string timestamp = getCurrentTimestamp();
    std::cout << DIM << "[" << timestamp << "]" << RESET 
              << "  " << PINK << "[INFO ]" << RESET 
              << "  " << PINK << message << RESET << std::endl;
}

void Logger::connection(const std::string& message) {
    std::string timestamp = getCurrentTimestamp();
    std::cout << DIM << "[" << timestamp << "]" << RESET 
              << "  " << BLUE << "[INFO ]" << RESET 
              << "  " << BLUE << message << RESET << std::endl;
}

void Logger::request(const std::string& method, const std::string& uri, int socket_fd) {
    std::string timestamp = getCurrentTimestamp();
    std::ostringstream oss;
    oss << "Request Received From Socket " << socket_fd << ", Method=<" << method << "> URI=<" << uri << ">";
    
    std::cout << DIM << "[" << timestamp << "]" << RESET 
              << "  " << CYAN << "[INFO ]" << RESET 
              << "  " << CYAN << oss.str() << RESET << std::endl;
}

void Logger::response(int status_code, int socket_fd) {
    std::string timestamp = getCurrentTimestamp();
    std::ostringstream oss;
    oss << "Response Sent To Socket " << socket_fd << ", Status=<" << status_code << ">";
    
    std::cout << DIM << "[" << timestamp << "]" << RESET 
              << "  " << GREEN << "[INFO ]" << RESET 
              << "  " << GREEN << oss.str() << RESET << std::endl;
}

void Logger::timeout(int socket_fd) {
    std::string timestamp = getCurrentTimestamp();
    std::ostringstream oss;
    oss << "Client " << socket_fd << " Timeout, Closing Connection..";
    
    std::cout << DIM << "[" << timestamp << "]" << RESET 
              << "  " << PINK << "[INFO ]" << RESET 
              << "  " << PINK << oss.str() << RESET << std::endl;
}