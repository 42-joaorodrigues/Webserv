#include "Response.hpp"
#include <ctime>
#include <iomanip>


std::string getCurrentDate()
{
    std::ostringstream oss;
    std::time_t t = std::time(0);
    std::tm tm = *std::gmtime(&t);
    oss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
    return oss.str();
}
std::string Response::toString() const //create response
{
    std::ostringstream response;
    response.http_version = request.http_version;

    response << http_version << " " << status_code << " " << status_message << "\r\n";

    //headers
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {
        response << it->first << ": " << it->second << "\r\n";
    }//iterate through the map headers and store it

    if (headers.find("Content-Length") == headers.end() && headers.find("content-length") == headers.end())
        response << "Content-Length: " << body.size() << "\r\n";
    response << "\r\n";
    response << body; //body (html)
    return response.str();
}

Response::Response() : status_code(200), status_message("OK"), body(""){};

void Response::setStatus(int code, const std::string &message)
{
    status_code = code;
    status_message = message;
}


void Response::setBody(const std::string &content, const std::string &contentType)
{
    this->body = content;

}