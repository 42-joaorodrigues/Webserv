#ifndef COOKIE_HPP
#define COOKIE_HPP




#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>  

class Cookie
{
    public:
        std::string GenCookieId(int length);
        std::string CreateCookieRequest(const std::string &name,
                                        const std::string &value,
                                        const std::string &path,
                                        int maxAge,
                                        bool HttpOnly,
                                        bool secure,
                                        const std::string &sameSite);



    private:

};


#endif