#ifndef COLABB_CONFIG_MANAGER_HPP
#define COLABB_CONFIG_MANAGER_HPP

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace colabb {
namespace infrastructure {

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();
    
    // Provider management
    std::string get_provider() const;
    void set_provider(const std::string& provider);
    
    // API key management (using libsecret)
    std::string get_api_key(const std::string& provider);
    void set_api_key(const std::string& provider, const std::string& key);
    
    // Save/load config
    void save();
    void load();
    
private:
    std::string config_path_;
    nlohmann::json config_;
    
    std::string get_config_dir();
    void ensure_config_dir();
};

} // namespace infrastructure
} // namespace colabb

#endif // COLABB_CONFIG_MANAGER_HPP
