#ifndef CGIUTIL_HPP
#define CGIUTIL_HPP

#include <string>
#include <map>

// Forward declarations
class Request;
struct LocationData;
struct MatchedLocation;

class CgiUtil {
public:
    static bool isCGIRequest(const std::string& filePath, const LocationData* location);
    static std::map<std::string, std::string> buildCGIEnvironment(const Request& req, const std::string& scriptPath, const MatchedLocation& matched);
};

#endif // CGIUTIL_HPP