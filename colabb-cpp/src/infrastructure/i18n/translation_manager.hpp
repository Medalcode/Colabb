#ifndef COLABB_TRANSLATION_MANAGER_HPP
#define COLABB_TRANSLATION_MANAGER_HPP

#include <string>
#include <unordered_map>
#include <memory>

namespace colabb {
namespace infrastructure {

class TranslationManager {
public:
    static TranslationManager& instance();
    
    void load_language(const std::string& lang_code);
    std::string get(const std::string& key);
    
    // Helper for formatted strings (simplistic)
    template<typename... Args>
    std::string get_fmt(const std::string& key, Args... args) {
        // In a real app, use fmt::format
        return get(key); 
    }

private:
    TranslationManager();
    
    std::string current_lang_;
    std::unordered_map<std::string, std::string> translations_;
    
    void load_en();
    void load_es();
};

} // namespace infrastructure
} // namespace colabb

#endif // COLABB_TRANSLATION_MANAGER_HPP
