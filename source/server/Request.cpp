#include "Request.hpp"


bool Request::parse(const std::string &raw_request) 
{
    std::istringstream raw_request_stream(raw_request);
    std::string line;

    if (!std::getline(raw_request_stream, line) || line.empty())
        return false;
    std::istringstream first_line(line);

    if (!(first_line >> method >> uri >> http_version))
        return false;

    while (std::getline())
    {
        
    }

}

