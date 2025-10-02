#ifndef DIRECTORYLISTING_HPP
#define DIRECTORYLISTING_HPP

#include <string>

class DirectoryListing {
public:
    static std::string generate(const std::string& directoryPath, const std::string& requestUri);
};

#endif // DIRECTORYLISTING_HPP