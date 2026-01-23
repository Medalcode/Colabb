#include <gtest/gtest.h>
#include "infrastructure/config/config_manager.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using namespace colabb::infrastructure;

class ConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a unique temporary directory for this test run
        temp_dir_ = fs::temp_directory_path() / ("colabb_test_" + std::to_string(std::rand()));
        fs::create_directories(temp_dir_);
        
        // Define config path
        config_path_ = temp_dir_ / "config.json";
    }

    void TearDown() override {
        fs::remove_all(temp_dir_);
    }

    fs::path temp_dir_;
    fs::path config_path_;
};

// We need a way to inject the config path into ConfigManager to test it safely.
// Since ConfigManager uses hardcoded paths in the constructor, we might need to modify it 
// or subclass it for testing if possible.
// 
// Looking at the implementation, ConfigManager determines path in constructor.
// To make it testable, we should add a constructor that accepts a path, or a protected method.
//
// For this test, I will assume we modified ConfigManager to accept a path or I will hack it.
// 
// WAIT: The ConfigManager uses `get_config_path()` private method.
// I can't easily change the path without refactoring.
//
// STRATEGY: 
// 1. Refactor ConfigManager to accept a config path in constructor (optional).
// 2. OR Set HOME env var to temp dir? (might affect other things)
// 3. OR Use a "TestConfigManager" that inherits if virtual.
//
// Let's go with setting HOME env var, it's a common trick. 
// But ConfigManager uses `g_get_user_config_dir()`, which respects XDG_CONFIG_HOME.
// So setting XDG_CONFIG_HOME is safer.

TEST_F(ConfigManagerTest, LoadDefaults) {
    // Set XDG_CONFIG_HOME to our temp dir
    setenv("XDG_CONFIG_HOME", temp_dir_.c_str(), 1);
    
    ConfigManager config;
    // It should create the directory and file? Or just load defaults.
    // get_provider() should return "groq" by default
    EXPECT_EQ(config.get_provider(), "groq");
}

TEST_F(ConfigManagerTest, SaveAndLoad) {
    setenv("XDG_CONFIG_HOME", temp_dir_.c_str(), 1);
    
    {
        ConfigManager config;
        config.set_provider("openai");
        // Destructor usually saves? No, check save() method availability.
        // It saves on set_provider? implementation says:
        // void ConfigManager::set_provider(const std::string& provider) {
        //    provider_ = provider;
        //    save_config();
        // }
        // So yes.
    }
    
    // Reload
    {
        ConfigManager config;
        EXPECT_EQ(config.get_provider(), "openai");
    }
}

// Secret storage tests are harder because they require a secret service (D-Bus).
// In a CI/headless environment, this might fail or hang.
// We'll trust the libsecret logic for now or mock it if we were rigorous.
