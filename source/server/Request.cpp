#include "Request.hpp"

Request::Request()
    : method(""), uri(""), http_version(""), body("") {}

bool Request::isValid()
{
    if (method != "GET" && method != "POST" && method != "DELETE")
    {
        std::cerr << "Unsupported Method: " << method << std::endl;
        return false;
    }
    if (http_version != "HTTP/1.1")
    {
        std::cerr << "Wrong Version: " << http_version << std::endl;
        return false;
    }
    if (headers.find("host") == headers.end())
    {
        std::cerr << "Host not found" << std::endl;
        return false;
    }
    if (headers.find("content-length") != headers.end())
    {
        const std::string &lenStr = headers["Content-Length"];
        for (size_t i = 0; i < lenStr.size(); i++)
        {
            if (!std::isdigit(static_cast<unsigned char>(lenStr[i])))
            {
                std::cerr << "Error Content-Length needs to be a number" << std::endl;
                return false;
            }
            size_t len = std::atoi(lenStr.c_str());
            if (len != body.size())
            {
                std::cerr << "Content-Length mismatch (expected "
                    << len << ", got " << body.size() << ")" << std::endl;
                return false;
            }
        }
    }
    return true;
}

bool Request::parse(const std::string &raw_request) 
{
    std::istringstream raw_request_stream(raw_request);
    std::string line;
    std::string name;
    std::string value;

    if (!std::getline(raw_request_stream, line) || line == "\r")
        return false;
    std::istringstream first_line(line);
    if (!(first_line >> method >> uri >> http_version))
        return false;
    while (std::getline(raw_request_stream, line) && line != "\r")
    {
        size_t pos = line.find(":");
        if (pos == std::string::npos)
            return false;
        name = line.substr(0, pos);
        value = line.substr(pos + 1);
        if (!value.empty() && value[0] == ' ')
            value.erase(0, 1);
        if (!value.empty() && value[value.size() - 1] == '\r')
            value.erase(value.size() - 1);
        for (size_t  i = 0; i < name.size(); i++)
            name[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(name[i])));
        headers[name] = value;
    }
    body.clear();
    while (std::getline(raw_request_stream, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        body += line + "\n";
    }
    return true;
}

