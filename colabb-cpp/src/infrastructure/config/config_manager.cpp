#include "infrastructure/config/config_manager.hpp"
#include <libsecret/secret.h>
#include <fstream>
#include <sys/stat.h>
#include <cstdlib>

namespace colabb {
namespace infrastructure {

static const SecretSchema colabb_schema = {
    "com.colabb.ApiKey",
    SECRET_SCHEMA_NONE,
    {
        { "provider", SECRET_SCHEMA_ATTRIBUTE_STRING },
        { nullptr, SECRET_SCHEMA_ATTRIBUTE_STRING }
    }
};

ConfigManager::ConfigManager() {
    ensure_config_dir();
    config_path_ = get_config_dir() + "/config.json";
    load();
}

ConfigManager::~ConfigManager() {
    save();
}

std::string ConfigManager::get_config_dir() {
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

void ConfigManager::ensure_config_dir() {
    std::string dir = get_config_dir();
    mkdir(dir.c_str(), 0755);
}

std::string ConfigManager::get_provider() const {
    if (config_.contains("provider")) {
        return config_["provider"].get<std::string>();
    }
    return "groq";
}

void ConfigManager::set_provider(const std::string& provider) {
    config_["provider"] = provider;
}

std::string ConfigManager::get_api_key(const std::string& provider) {
    GError* error = nullptr;
    gchar* password = secret_password_lookup_sync(
        &colabb_schema,
        nullptr,
        &error,
        "provider", provider.c_str(),
        nullptr
    );
    
    if (error) {
        g_error_free(error);
        return "";
    }
    
    if (!password) {
        return "";
    }
    
    std::string result(password);
    secret_password_free(password);
    
    return result;
}

void ConfigManager::set_api_key(const std::string& provider, const std::string& key) {
    GError* error = nullptr;
    secret_password_store_sync(
        &colabb_schema,
        SECRET_COLLECTION_DEFAULT,
        ("Colabb API Key: " + provider).c_str(),
        key.c_str(),
        nullptr,
        &error,
        "provider", provider.c_str(),
        nullptr
    );
    
    if (error) {
        g_printerr("Failed to store API key: %s\n", error->message);
        g_error_free(error);
    }
}

void ConfigManager::save() {
    std::ofstream file(config_path_);
    if (file.is_open()) {
        file << config_.dump(2);
    }
}

void ConfigManager::load() {
    std::ifstream file(config_path_);
    if (file.is_open()) {
        try {
            file >> config_;
        } catch (...) {
            config_ = nlohmann::json::object();
        }
    } else {
        config_ = nlohmann::json::object();
    }
}

} // namespace infrastructure
} // namespace colabb
