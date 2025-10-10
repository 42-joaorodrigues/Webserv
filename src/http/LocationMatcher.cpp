#include "LocationMatcher.hpp"
#include <algorithm>
#include <iostream>

// Main function to find the best matching location
MatchedLocation LocationMatcher::findMatchingLocation(const std::string& uri, const Server& server) {
    MatchedLocation result;
    std::string clean_uri = cleanUri(uri);
    
    const std::map<std::string, LocationData>& locations = server.getLocations();
    
    // Variables to track best match
    int bestScore = -1;
    std::string bestPath = "";
    const LocationData* bestLocation = NULL;
    
    // Iterate through all location blocks
    for (std::map<std::string, LocationData>::const_iterator it = locations.begin(); 
         it != locations.end(); ++it) {
        
        const std::string& location_path = it->first;
        const LocationData& location_data = it->second;
        
        int score = getMatchScore(location_path, clean_uri);
        
        if (score > bestScore) {
            bestScore = score;
            bestPath = location_path;
            bestLocation = &location_data;
        }
    }
    
    // Set result
    if (bestLocation != NULL) {
        result.location = bestLocation;
        result.matched_path = bestPath;
        result.effective_root = resolveEffectiveRoot(bestLocation, server);
        result.effective_indexes = resolveEffectiveIndexes(bestLocation, server);
        result.is_alias = !bestLocation->alias.empty();
    } else {
        // No location matched, use server defaults
        result.location = NULL;
        result.matched_path = "/";
        result.effective_root = server.getRoot();
        result.effective_indexes = server.getIndexes();
        result.is_alias = false;
    }
    
    return result;
}

// Calculate match score for a location path against URI
// Higher score = better match
// Returns -1 for no match
// Only handles simple prefix matching
int LocationMatcher::getMatchScore(const std::string& location_path, const std::string& uri) {
    // Simple prefix match - longer matches have higher priority
    if (matchesPrefix(uri, location_path)) {
        return location_path.length(); // Length determines specificity for prefix matches
    }
    
    return -1; // No match
}

// Check for simple prefix match
bool LocationMatcher::matchesPrefix(const std::string& uri, const std::string& location_path) {
    // Location path must be a prefix of the URI
    if (location_path.length() > uri.length()) {
        return false;
    }
    
    // Check if URI starts with location_path
    if (uri.substr(0, location_path.length()) != location_path) {
        return false;
    }
    
    // For proper prefix matching, either:
    // 1. location_path ends with '/'
    // 2. URI character after location_path is '/' or end of string
    // 3. Exact match (location_path == uri)
    if (location_path.length() == uri.length()) {
        return true; // Exact match via prefix logic
    }
    
    if (location_path[location_path.length() - 1] == '/') {
        return true;
    }
    
    if (uri[location_path.length()] == '/') {
        return true;
    }
    
    return false;
}

// Resolve the effective root directory
std::string LocationMatcher::resolveEffectiveRoot(const LocationData* location, const Server& server) {
    if (location != NULL) {
        // Alias takes precedence over root
        if (!location->alias.empty()) {
            return location->alias;
        }
        if (!location->root.empty()) {
            return location->root;
        }
    }
    return server.getRoot();
}

// Resolve the effective index files
std::vector<std::string> LocationMatcher::resolveEffectiveIndexes(const LocationData* location, const Server& server) {
    if (location != NULL && !location->_indexes.empty()) {
        return location->_indexes;
    }
    return server.getIndexes();
}

// Clean and normalize URI
std::string LocationMatcher::cleanUri(const std::string& uri) {
    std::string clean = uri;
    
    // Remove query string if present
    size_t query_pos = clean.find('?');
    if (query_pos != std::string::npos) {
        clean = clean.substr(0, query_pos);
    }
    
    // Remove fragment if present
    size_t fragment_pos = clean.find('#');
    if (fragment_pos != std::string::npos) {
        clean = clean.substr(0, fragment_pos);
    }
    
    // Ensure URI starts with /
    if (clean.empty() || clean[0] != '/') {
        clean = "/" + clean;
    }
    
    // Remove duplicate slashes
    std::string result;
    bool prev_slash = false;
    for (size_t i = 0; i < clean.length(); i++) {
        if (clean[i] == '/') {
            if (!prev_slash) {
                result += clean[i];
                prev_slash = true;
            }
        } else {
            result += clean[i];
            prev_slash = false;
        }
    }
    
    // Remove trailing slash unless it's root
    if (result.length() > 1 && result[result.length() - 1] == '/') {
        result.erase(result.length() - 1);
    }
    
    return result;
}

// Helper to build correct filesystem path based on alias vs root
std::string LocationMatcher::buildFilesystemPath(const std::string& requestedPath, const MatchedLocation& matched) {
    if (matched.is_alias) {
        // For alias: remove the matched location path from requestedPath and append the rest
        std::string remainingPath = requestedPath;
        
        // If requestedPath starts with the matched_path, remove it
        if (requestedPath.length() >= matched.matched_path.length() && 
            requestedPath.substr(0, matched.matched_path.length()) == matched.matched_path) {
            remainingPath = requestedPath.substr(matched.matched_path.length());
        }
        
        // Ensure no double slashes
        if (!remainingPath.empty() && remainingPath[0] == '/') {
            return matched.effective_root + remainingPath;
        } else if (!matched.effective_root.empty() && matched.effective_root[matched.effective_root.length() - 1] == '/') {
            return matched.effective_root + remainingPath;
        } else {
            return matched.effective_root + "/" + remainingPath;
        }
    } else {
        // For root: normal behavior - just append the full requested path
        return matched.effective_root + requestedPath;
    }
}