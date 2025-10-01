#ifndef UPLOADSERVICE_HPP
#define UPLOADSERVICE_HPP

#include <string>
#include <vector>

// Forward declarations
class Request;
class Server;
struct LocationData;

// Structure to hold uploaded file information
struct UploadedFile {
    std::string filename;
    std::string contentType;
    std::string content;
};

class UploadService {
public:
    static std::string parseMultipartBoundary(const std::string& contentType);
    static std::string extractFilename(const std::string& contentDisposition);
    static std::vector<UploadedFile> parseMultipartFormData(const std::string& body, const std::string& boundary);
    static size_t parseFileSize(const std::string& sizeStr);
    static bool isUploadAllowed(const LocationData* location, const Request& req, const Server& server);
    static bool createDirectoryRecursive(const std::string& path);
    static bool saveUploadedFile(const UploadedFile& file, const std::string& uploadDir);
};

#endif // UPLOADSERVICE_HPP