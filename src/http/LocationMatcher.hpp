#ifndef LOCATION_MATCHER_HPP
#define LOCATION_MATCHER_HPP

#include "Server.hpp"
#include <string>
#include <utility>

// Structure to hold matched location info
struct MatchedLocation {
    const LocationData* location;  // Pointer to matched location (NULL if using server defaults)
    std::string matched_path;      // The location path that was matched
    std::string effective_root;    // Resolved root directory to use
    std::vector<std::string> effective_indexes; // Resolved index files to use
    
    MatchedLocation() : location(NULL) {}
};

class LocationMatcher {
public:
    // Main function to find matching location
    static MatchedLocation findMatchingLocation(const std::string& uri, const Server& server);
    
private:
    // Helper functions for simple prefix matching
    static bool matchesPrefix(const std::string& uri, const std::string& location_path);
    
    // Helper to determine match score (based on prefix length)
    static int getMatchScore(const std::string& location_path, const std::string& uri);
    
    // Helper to build effective paths
    static std::string resolveEffectiveRoot(const LocationData* location, const Server& server);
    static std::vector<std::string> resolveEffectiveIndexes(const LocationData* location, const Server& server);
    
    // Helper to clean and normalize URI
    static std::string cleanUri(const std::string& uri);
};

#endif // LOCATION_MATCHER_HPP