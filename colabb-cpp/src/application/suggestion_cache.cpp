#include "application/suggestion_cache.hpp"
#include <algorithm>
#include <cctype>

namespace colabb {
namespace application {

SuggestionCache::SuggestionCache(size_t max_size, std::chrono::minutes ttl)
    : max_size_(max_size)
    , ttl_(ttl) {
}

void SuggestionCache::put(const std::string& query, const domain::Suggestion& suggestion) {
    std::string normalized = normalize_query(query);
    
    // Evict if cache is full
    if (cache_.size() >= max_size_) {
        evict_oldest();
    }
    
    CacheEntry entry;
    entry.suggestion = suggestion;
    entry.timestamp = std::chrono::system_clock::now();
    
    cache_[normalized] = entry;
}

std::optional<domain::Suggestion> SuggestionCache::get(const std::string& query) {
    std::string normalized = normalize_query(query);
    
    auto it = cache_.find(normalized);
    if (it == cache_.end()) {
        return std::nullopt;
    }
    
    // Check if expired
    if (is_expired(it->second)) {
        cache_.erase(it);
        return std::nullopt;
    }
    
    return it->second.suggestion;
}

void SuggestionCache::clear() {
    cache_.clear();
}

void SuggestionCache::evict_oldest() {
    if (cache_.empty()) {
        return;
    }
    
    auto oldest = cache_.begin();
    for (auto it = cache_.begin(); it != cache_.end(); ++it) {
        if (it->second.timestamp < oldest->second.timestamp) {
            oldest = it;
        }
    }
    
    cache_.erase(oldest);
}

bool SuggestionCache::is_expired(const CacheEntry& entry) const {
    auto now = std::chrono::system_clock::now();
    auto age = std::chrono::duration_cast<std::chrono::minutes>(now - entry.timestamp);
    return age > ttl_;
}

std::string SuggestionCache::normalize_query(const std::string& query) const {
    std::string normalized = query;
    
    // Convert to lowercase
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
        [](unsigned char c) { return std::tolower(c); });
    
    // Trim whitespace
    normalized.erase(0, normalized.find_first_not_of(" \t\n\r"));
    normalized.erase(normalized.find_last_not_of(" \t\n\r") + 1);
    
    return normalized;
}

} // namespace application
} // namespace colabb
