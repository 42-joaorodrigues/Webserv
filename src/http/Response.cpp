#include "Response.hpp"
#include "../cookie/cookie.hpp"
#include <ctime>
#include <iomanip>


std::string getCurrentDate()
{
    char buffer[100];
    std::time_t t = std::time(0);
    std::tm *tm = std::gmtime(&t);
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", tm);
    return std::string(buffer);
}



std::string Response::toString() const //create response
{
    std::ostringstream response;


    response << "HTTP/1.1" << " " << status_code << " " << status_message << "\r\n";

    if (headers.find("Date") == headers.end())
        response << "Date: " << getCurrentDate() << "\r\n";

    if (headers.find("Server") == headers.end())
        response << "Server: Webserver/1.0\r\n";
    
    // Add cookies first
    for (std::vector<std::string>::const_iterator it = cookies.begin(); it != cookies.end(); ++it)
    {
        response << "Set-Cookie: " << *it << "\r\n";
    }
    
    //headers
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {
        response << it->first << ": " << it->second << "\r\n";
    }//iterate through the map headers and store it
    if (headers.find("Content-Length") == headers.end() && 
        headers.find("content-length") == headers.end())
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
    headers["Content-Type"] = contentType;
}

void Response::addCookie(const std::string &cookieStr)
{
    cookies.push_back(cookieStr);
}
void Response::setCookie(const std::string &name, const std::string &value, 
                       const std::string &path, int maxAge, 
                       bool httpOnly, bool secure,
                       const std::string &sameSite)
{
    std::ostringstream cookie;
    
    // Basic name=value pair
    cookie << name << "=" << value;
    
    if (!path.empty())
        cookie << "; Path=" << path;
        
    if (maxAge >= 0)
        cookie << "; Max-Age=" << maxAge;
        
    if (httpOnly)
        cookie << "; HttpOnly";
        
    if (secure)
        cookie << "; Secure";
        
    if (!sameSite.empty())
        cookie << "; SameSite=" << sameSite;
    
    // Add the cookie to our vector
    cookies.push_back(cookie.str());
}