#ifndef COLABB_PROFILE_MANAGER_HPP
#define COLABB_PROFILE_MANAGER_HPP

#include "domain/models/terminal_profile.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <nlohmann/json.hpp>

namespace colabb {
namespace infrastructure {

class ProfileManager {
public:
    ProfileManager();
    ~ProfileManager() = default;

    // Load/Save
    void load_profiles();
    void save_profiles();

    // Profiles access
    domain::TerminalProfile get_profile(const std::string& name);
    std::vector<std::string> get_profile_names() const;
    
    // CRUD
    void save_profile(const domain::TerminalProfile& profile);
    void delete_profile(const std::string& name);

    // Default profile
    std::string get_default_profile_name() const;
    void set_default_profile(const std::string& name);

private:
    std::unordered_map<std::string, domain::TerminalProfile> profiles_;
    std::string default_profile_name_;
    std::string config_path_;

    std::string get_config_dir();
    void create_defaults_if_empty();
};

} // namespace infrastructure
} // namespace colabb

#endif // COLABB_PROFILE_MANAGER_HPP
