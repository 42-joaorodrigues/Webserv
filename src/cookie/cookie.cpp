#include "cookie.hpp"




std::string Cookie::GenCookieId(int length)
{
    static const char alphanum[] =
        "0123456789"
        "abcdefghijklmnopqrtuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";


    std::string resultid;

    for (int i = 0;i < length;i++)
    {
        resultid += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return resultid;
}


std::string Cookie::CreateCookieRequest(const std::string &name,
                                        const std::string &value,
                                        const std::string &path,
                                        int maxAge,
                                        bool HttpOnly,
                                        bool secure,
                                        const std::string &sameSite)
{
   std::ostringstream cookie;


    cookie << name << "=" << value;
    if (!path.empty()) cookie << "; Path=" << path;
    if (maxAge >= 0) cookie << "; Max-Age=" << maxAge;
    if (HttpOnly) cookie << "; HttpOnly";
    if (secure) cookie << "; Secure";
    if (!sameSite.empty()) cookie << "; SameSite=" << sameSite;

    return cookie.str();
}