#include "Request.hpp"

Request::Request()
    : method(""), uri(""), http_version(""), body("") {}


bool Request::isValidChunkSizeLine(const std::string &line)
{
    if (line.empty())
        return false;
    size_t semicoln = line.find(';');
    std::string sizePart;
    if (semicoln != std::string::npos)
    {
        sizePart = line.substr(0, semicoln);
    }
    else
    {
        sizePart = line;
    }
    for (size_t i = 0; i < sizePart.length();i++)
    {
        if (!isxdigit(sizePart[i]))
        {
            std::cout << sizePart[i] << std::endl;
            return false;
        }
    }
    return true;
}
std::string Request::dechunk(const std::string &chunked)
{
    std::string line;
    std::istringstream iss(chunked);
    std::string result;

    while (std::getline(iss, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        if (line.empty())
            continue;
        if (!isValidChunkSizeLine(line))
            throw std::runtime_error("Chunk size isn't valid");
        size_t chunk_size = strtol(line.c_str(), 0, 16);
        if (chunk_size == 0)
            break;
        std::string buffer(chunk_size, '\0');
        iss.read(&buffer[0], chunk_size);
        result.append(buffer, 0, iss.gcount());
        iss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::cout << result;
    return result;
}

bool Request::isValid()
{
    const int hasTE = headers.find("transfer-encoding") != headers.end();
    const int hasCL = headers.find("content-length") != headers.end();
    const int IsHostAbs = headers.find("host") == headers.end();
    
    if (hasTE && hasCL)
    {
        std::cerr << "Conflict: both Transfer-Encoding and Content-Length\n";
        return false;
    }
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
    if (IsHostAbs)
    {
        std::cerr << "Host not found" << std::endl;
        return false;
    }
    if (hasCL)
    {
        const std::string &lenStr = headers["content-length"];
        for (size_t i = 0; i < lenStr.size(); i++)
        {
            if (!std::isdigit(static_cast<unsigned char>(lenStr[i])))
            {
                std::cerr << "Error Content-Length needs to be a number" << std::endl;
                return false;
            }
        }
        size_t len = std::atoi(lenStr.c_str());
        if (len != body.size())
        {
            std::cerr << "Content-Length mismatch (expected "
                << len << ", got " << body.size() << ")" << std::endl;
                return false;
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
    std::string rest((std::istreambuf_iterator<char>(raw_request_stream)),
                    std::istreambuf_iterator<char>());
    body = rest;
    if ((headers.find("transfer-encoding") != headers.end()
        && headers["transfer-encoding"] == "chunked") ||
		(headers.find("Transfer-Encoding") != headers.end()
        && headers["Transfer-Encoding"] == "chunked"))
    {
        try {
            body = dechunk(body);
            }
        catch (const std::exception& e)
        {
            std::cerr << "Chunked body parse error\n" << e.what() << std::endl;
            return false;
        }
    }
    else if (headers.find("content-length") != headers.end())
    {
        body = body.substr(0, atoi(headers["content-length"].c_str()));
    }
    return true;
}

