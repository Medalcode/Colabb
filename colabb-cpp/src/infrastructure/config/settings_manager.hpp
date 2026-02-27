#ifndef COLABB_SETTINGS_MANAGER_HPP
#define COLABB_SETTINGS_MANAGER_HPP

#include "domain/models/terminal_profile.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace colabb {
namespace infrastructure {

/**
 * @brief Unified Manager for Application Settings and User Profiles.
 * Consolidates ConfigManager and ProfileManager.
 */
class SettingsManager {
public:
    SettingsManager();
    ~SettingsManager();

    // --- General App Settings (Old ConfigManager) ---
    std::string get_ai_provider() const;
    void set_ai_provider(const std::string& provider);
    
    // API Key management (Secret Storage)
    std::string get_api_key(const std::string& provider);
    void set_api_key(const std::string& provider, const std::string& key);

    // --- Profile Management (Old ProfileManager) ---
    domain::TerminalProfile get_profile(const std::string& name);
    std::vector<std::string> get_profile_names() const;
    void save_profile(const domain::TerminalProfile& profile);
    void delete_profile(const std::string& name);
    
    std::string get_default_profile_name() const;
    void set_default_profile(const std::string& name);

    // Persistence
    void sync();

private:
    // Memory state
    nlohmann::json settings_root_;
    std::unordered_map<std::string, domain::TerminalProfile> profiles_;
    std::string default_profile_name_;

    // Paths
    std::string get_base_config_dir();
    std::string settings_file_path_;
    
    // Internal Loads
    void load_all();
    void save_all();
    void ensure_config_dir();
    void create_default_profiles();
};

} // namespace infrastructure
} // namespace colabb

#endif // COLABB_SETTINGS_MANAGER_HPP
