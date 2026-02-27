#include "infrastructure/config/settings_manager.hpp"
#include <libsecret/secret.h>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <cstdlib>

namespace fs = std::filesystem;

namespace colabb {
namespace domain {
    // Re-utilizamos las funciones de serialización de TerminalProfile
    void to_json(nlohmann::json& j, const TerminalProfile& p);
    void from_json(const nlohmann::json& j, TerminalProfile& p);
}

namespace infrastructure {

static const SecretSchema colabb_schema = {
    "com.colabb.ApiKey",
    SECRET_SCHEMA_NONE,
    {
        { "provider", SECRET_SCHEMA_ATTRIBUTE_STRING },
        { nullptr, SECRET_SCHEMA_ATTRIBUTE_STRING }
    }
};

SettingsManager::SettingsManager() {
    ensure_config_dir();
    settings_file_path_ = get_base_config_dir() + "/settings.json";
    load_all();
}

SettingsManager::~SettingsManager() {
    save_all();
}

std::string SettingsManager::get_base_config_dir() {
    const char* config_home = getenv("XDG_CONFIG_HOME");
    if (config_home) return std::string(config_home) + "/colabb";
    
    const char* home = getenv("HOME");
    if (home) return std::string(home) + "/.config/colabb";
    
    return "/tmp/colabb";
}

void SettingsManager::ensure_config_dir() {
    fs::create_directories(get_base_config_dir());
}

// --- Persistence ---

void SettingsManager::load_all() {
    profiles_.clear();
    if (!fs::exists(settings_file_path_)) {
        settings_root_ = nlohmann::json::object();
        create_default_profiles();
        return;
    }

    try {
        std::ifstream file(settings_file_path_);
        file >> settings_root_;

        // Load Profiles
        if (settings_root_.contains("profiles") && settings_root_["profiles"].is_array()) {
            for (const auto& jp : settings_root_["profiles"]) {
                auto p = jp.get<domain::TerminalProfile>();
                profiles_[p.name] = p;
            }
        }

        // Default profile name
        default_profile_name_ = settings_root_.value("default_profile", "Default");

    } catch (const std::exception& e) {
        std::cerr << "Settings error: " << e.what() << std::endl;
        create_default_profiles();
    }

    if (profiles_.empty()) create_default_profiles();
}

void SettingsManager::save_all() {
    // Sync current profiles map to JSON
    std::vector<domain::TerminalProfile> profile_list;
    for (const auto& [name, p] : profiles_) profile_list.push_back(p);
    
    settings_root_["profiles"] = profile_list;
    settings_root_["default_profile"] = default_profile_name_;

    std::ofstream file(settings_file_path_);
    if (file.is_open()) {
        file << settings_root_.dump(4);
    }
}

void SettingsManager::sync() {
    save_all();
}

// --- App Settings ---

std::string SettingsManager::get_ai_provider() const {
    return settings_root_.value("ai_provider", "groq");
}

void SettingsManager::set_ai_provider(const std::string& provider) {
    settings_root_["ai_provider"] = provider;
    save_all();
}

std::string SettingsManager::get_api_key(const std::string& provider) {
    GError* error = nullptr;
    gchar* password = secret_password_lookup_sync(&colabb_schema, nullptr, &error, "provider", provider.c_str(), nullptr);
    if (error) { g_error_free(error); return ""; }
    if (!password) return "";
    std::string result(password);
    secret_password_free(password);
    return result;
}

void SettingsManager::set_api_key(const std::string& provider, const std::string& key) {
    GError* error = nullptr;
    secret_password_store_sync(&colabb_schema, SECRET_COLLECTION_DEFAULT, 
        ("Colabb API Key: " + provider).c_str(), key.c_str(), nullptr, &error, "provider", provider.c_str(), nullptr);
    if (error) { g_printerr("Secret error: %s\n", error->message); g_error_free(error); }
}

// --- Profile Logic ---

domain::TerminalProfile SettingsManager::get_profile(const std::string& name) {
    if (profiles_.count(name)) return profiles_[name];
    if (profiles_.count(default_profile_name_)) return profiles_[default_profile_name_];
    return domain::TerminalProfile::create_default();
}

std::vector<std::string> SettingsManager::get_profile_names() const {
    std::vector<std::string> names;
    for (const auto& [name, p] : profiles_) names.push_back(name);
    return names;
}

void SettingsManager::save_profile(const domain::TerminalProfile& profile) {
    profiles_[profile.name] = profile;
    save_all();
}

void SettingsManager::delete_profile(const std::string& name) {
    if (profiles_.size() <= 1) return;
    profiles_.erase(name);
    if (default_profile_name_ == name) default_profile_name_ = profiles_.begin()->first;
    save_all();
}

std::string SettingsManager::get_default_profile_name() const {
    return default_profile_name_;
}

void SettingsManager::set_default_profile(const std::string& name) {
    if (profiles_.count(name)) {
        default_profile_name_ = name;
        save_all();
    }
}

void SettingsManager::create_default_profiles() {
    auto d = domain::TerminalProfile::create_default();
    auto l = domain::TerminalProfile::create_light();
    profiles_[d.name] = d;
    profiles_[l.name] = l;
    default_profile_name_ = d.name;
    save_all();
}

} // namespace infrastructure
} // namespace colabb
