#ifndef COLABB_SUGGESTION_CACHE_HPP
#define COLABB_SUGGESTION_CACHE_HPP

#include "domain/models/suggestion.hpp"
#include <string>
#include <unordered_map>
#include <chrono>
#include <optional>

namespace colabb {
namespace application {

class SuggestionCache {
public:
    SuggestionCache(size_t max_size = 100, std::chrono::minutes ttl = std::chrono::minutes(30));
    
    void put(const std::string& query, const domain::Suggestion& suggestion);
    std::optional<domain::Suggestion> get(const std::string& query);
    void clear();
    size_t size() const { return cache_.size(); }
    
private:
    struct CacheEntry {
        domain::Suggestion suggestion;
        std::chrono::system_clock::time_point timestamp;
    };
    
    std::unordered_map<std::string, CacheEntry> cache_;
    size_t max_size_;
    std::chrono::minutes ttl_;
    
    void evict_oldest();
    bool is_expired(const CacheEntry& entry) const;
    std::string normalize_query(const std::string& query) const;
};

} // namespace application
} // namespace colabb

#endif // COLABB_SUGGESTION_CACHE_HPP
