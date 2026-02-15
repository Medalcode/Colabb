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
    auto it = cache_.find(normalized);

    // Update existing entry
    if (it != cache_.end()) {
        it->second.suggestion = suggestion;
        it->second.timestamp = std::chrono::system_clock::now();
        // Move key to front (most recently used)
        lru_list_.splice(lru_list_.begin(), lru_list_, it->second.lru_it);
        it->second.lru_it = lru_list_.begin();
        return;
    }

    // Evict if cache is full
    if (cache_.size() >= max_size_) {
        evict_oldest();
    }

    // Insert new entry at front
    lru_list_.push_front(normalized);
    CacheEntry entry;
    entry.suggestion = suggestion;
    entry.timestamp = std::chrono::system_clock::now();
    entry.lru_it = lru_list_.begin();
    cache_[normalized] = std::move(entry);
}

std::optional<domain::Suggestion> SuggestionCache::get(const std::string& query) {
    std::string normalized = normalize_query(query);
    auto it = cache_.find(normalized);
    if (it == cache_.end()) {
        return std::nullopt;
    }

    // Check if expired
    if (is_expired(it->second)) {
        // remove from lru list and map
        lru_list_.erase(it->second.lru_it);
        cache_.erase(it);
        return std::nullopt;
    }

    // Move to front as most recently used
    lru_list_.splice(lru_list_.begin(), lru_list_, it->second.lru_it);
    it->second.lru_it = lru_list_.begin();

    return it->second.suggestion;
}

void SuggestionCache::clear() {
    cache_.clear();
    lru_list_.clear();
}

void SuggestionCache::evict_oldest() {
    if (lru_list_.empty()) {
        return;
    }

    // Least recently used is at the back of the list
    std::string key = lru_list_.back();
    lru_list_.pop_back();
    cache_.erase(key);
}

bool SuggestionCache::is_expired(const CacheEntry& entry) const {
    auto now = std::chrono::system_clock::now();
    auto age = std::chrono::duration_cast<std::chrono::minutes>(now - entry.timestamp);
    return age > ttl_;
}

std::string SuggestionCache::normalize_query(const std::string& query) const {
    std::string normalized;
    normalized.reserve(query.length());
    
    bool space_seen = false;
    for (char c : query) {
        if (std::isspace(c)) {
            if (!space_seen && !normalized.empty()) {
                normalized += ' ';
                space_seen = true;
            }
        } else {
            normalized += std::tolower(c);
            space_seen = false;
        }
    }
    
    // Trim trailing space if any (from the loop logic, last char might be space if input ended with spaces)
    // Wait, my logic adds space only if !space_seen. 
    // If input is "a   b  ", 
    // 'a': norm="a", space=false
    // ' ': !space && !empty -> norm="a ", space=true
    // ' ': space -> skip
    // 'b': norm="a b", space=false
    // ' ': !space && !empty -> norm="a b ", space=true
    
    if (!normalized.empty() && normalized.back() == ' ') {
        normalized.pop_back();
    }
    
    return normalized;
}

} // namespace application
} // namespace colabb
