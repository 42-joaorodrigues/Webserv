#include "ErrorPage.hpp"

std::string ErrorPage::getdefaulterrorpage(int status) const
{
    switch (status)
    {
        case 400:
            return "<!DOCTYPE html><html><head><title>400 Bad Request</title></head>"
                         "<body><h1>400 Bad Request</h1><p>The server could not understand your request.</p></body></html>";
        case 403:
            return "<!DOCTYPE html><html><head><title>403 Forbidden</title></head>"
                        "<body><h1>403 Request failed due to insufficient permissions";
        case 404:
            return "<!DOCTYPE html><html><head><title>404 Page Not Found</title></head>"
                        "<body><h1>The requested resource was not found on this server.";
        case 405: return "<!DOCTYPE html><html><head><title>405 Method Not Allowed</title></head>"
                         "<body><h1>405 Method Not Allowed</h1><p>The request method is not supported for this resource.</p></body></html>";
        case 413: return "<!DOCTYPE html><html><head><title>413 Payload Too Large</title></head>"
                         "<body><h1>413 Payload Too Large</h1><p>The request entity is too large.</p></body></html>";
        case 500: return "<!DOCTYPE html><html><head><title>500 Internal Server Error</title></head>"
                         "<body><h1>500 Internal Server Error</h1><p>The server encountered an internal error.</p></body></html>";
        case 501: return "<!DOCTYPE html><html><head><title>501 Not Implemented</title></head>"
                         "<body><h1>501 Not Implemented</h1><p>This method is not implemented by the server.</p></body></html>";
        case 505: return "<!DOCTYPE html><html><head><title>505 HTTP Version Not Supported</title></head>"
                         "<body><h1>505 HTTP Version Not Supported</h1><p>The server does not support this HTTP version.</p></body></html>";
        default:  return "<!DOCTYPE html><html><head><title>Error</title></head>"
                         "<body><h1>Unknown Error</h1></body></html>";
    }
}