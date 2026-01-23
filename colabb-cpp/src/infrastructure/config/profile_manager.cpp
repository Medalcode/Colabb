#include "infrastructure/config/profile_manager.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

// JSON serialization support must be in the same namespace as the type or in global
// We put it in colabb::domain where TerminalProfile is defined
namespace colabb {
namespace domain {

void to_json(nlohmann::json& j, const TerminalProfile& p) {
    j = nlohmann::json{
        {"name", p.name},
        {"background_color", p.background_color},
        {"foreground_color", p.foreground_color},
        {"palette", p.palette},
        {"font_family", p.font_family},
        {"font_size", p.font_size},
        {"startup_command", p.startup_command},
        {"scrollback_lines", p.scrollback_lines},
        {"cursor_blink", p.cursor_blink}
    };
}

void from_json(const nlohmann::json& j, TerminalProfile& p) {
    j.at("name").get_to(p.name);
    j.at("background_color").get_to(p.background_color);
    j.at("foreground_color").get_to(p.foreground_color);
    j.at("palette").get_to(p.palette);
    j.at("font_family").get_to(p.font_family);
    j.at("font_size").get_to(p.font_size);
    p.startup_command = j.value("startup_command", "");
    p.scrollback_lines = j.value("scrollback_lines", 10000);
    p.cursor_blink = j.value("cursor_blink", true);
}

} // namespace domain
} // namespace colabb

namespace colabb {
namespace infrastructure {

ProfileManager::ProfileManager() {
    config_path_ = get_config_dir() + "/profiles.json";
    load_profiles();
}

std::string ProfileManager::get_config_dir() {
    const char* config_home = getenv("XDG_CONFIG_HOME");
    if (config_home) {
        return std::string(config_home) + "/colabb";
    }
    const char* home = getenv("HOME");
    if (home) {
        return std::string(home) + "/.config/colabb";
    }
    return "/tmp/colabb";
}

void ProfileManager::load_profiles() {
    profiles_.clear();
    
    if (!fs::exists(config_path_)) {
        create_defaults_if_empty();
        return;
    }

    try {
        std::ifstream file(config_path_);
        nlohmann::json j;
        file >> j;

        if (j.contains("default_profile")) {
            default_profile_name_ = j["default_profile"];
        }

        if (j.contains("profiles") && j["profiles"].is_array()) {
            for (const auto& jp : j["profiles"]) {
                colabb::domain::TerminalProfile p = jp.get<colabb::domain::TerminalProfile>();
                profiles_[p.name] = p;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading profiles: " << e.what() << std::endl;
        create_defaults_if_empty(); // Fallback
    }

    if (profiles_.empty()) {
        create_defaults_if_empty();
    }
}

void ProfileManager::save_profiles() {
    nlohmann::json j;
    j["default_profile"] = default_profile_name_;
    
    std::vector<colabb::domain::TerminalProfile> profile_list;
    for (const auto& [name, p] : profiles_) {
        profile_list.push_back(p);
    }
    j["profiles"] = profile_list;

    // Ensure directory exists
    fs::path p(config_path_);
    fs::create_directories(p.parent_path());

    std::ofstream file(config_path_);
    file << j.dump(4);
}

void ProfileManager::create_defaults_if_empty() {
    auto default_p = colabb::domain::TerminalProfile::create_default();
    auto light_p = colabb::domain::TerminalProfile::create_light();

    profiles_[default_p.name] = default_p;
    profiles_[light_p.name] = light_p;
    
    default_profile_name_ = default_p.name;
    save_profiles();
}

colabb::domain::TerminalProfile ProfileManager::get_profile(const std::string& name) {
    if (profiles_.find(name) != profiles_.end()) {
        return profiles_[name];
    }
    // Fallback to default if not found
    if (profiles_.find(default_profile_name_) != profiles_.end()) {
        return profiles_[default_profile_name_];
    }
    // Ultimate fallback
    return colabb::domain::TerminalProfile::create_default();
}

std::vector<std::string> ProfileManager::get_profile_names() const {
    std::vector<std::string> names;
    for (const auto& [name, p] : profiles_) {
        names.push_back(name);
    }
    return names;
}

void ProfileManager::save_profile(const colabb::domain::TerminalProfile& profile) {
    profiles_[profile.name] = profile;
    save_profiles();
}

void ProfileManager::delete_profile(const std::string& name) {
    // Prevent deleting the last profile
    if (profiles_.size() <= 1) return;
    
    profiles_.erase(name);
    
    // If we deleted the default, reset default
    if (default_profile_name_ == name) {
        default_profile_name_ = profiles_.begin()->first;
    }
    
    save_profiles();
}

std::string ProfileManager::get_default_profile_name() const {
    return default_profile_name_;
}

void ProfileManager::set_default_profile(const std::string& name) {
    if (profiles_.count(name)) {
        default_profile_name_ = name;
        save_profiles();
    }
}

} // namespace infrastructure
} // namespace colabb
