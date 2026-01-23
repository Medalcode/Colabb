#include <gtest/gtest.h>
#include "infrastructure/config/profile_manager.hpp"
#include <filesystem>
#include <fstream>

using namespace colabb::infrastructure;
using namespace colabb::domain;

class ProfileManagerTest : public ::testing::Test {
protected:
    std::string temp_dir_;

    void SetUp() override {
        // Create unique temp directory
        temp_dir_ = "/tmp/colabb_test_profile_" + std::to_string(std::rand());
        std::filesystem::create_directories(temp_dir_);
        setenv("XDG_CONFIG_HOME", temp_dir_.c_str(), 1);
    }

    void TearDown() override {
        std::filesystem::remove_all(temp_dir_);
    }
};

TEST_F(ProfileManagerTest, DefaultsCreation) {
    ProfileManager manager;
    auto profiles = manager.get_profile_names();
    EXPECT_GE(profiles.size(), 2); // Default and Light
    
    EXPECT_EQ(manager.get_default_profile_name(), "Default");
}

TEST_F(ProfileManagerTest, CRUD) {
    ProfileManager manager;
    
    // Create
    auto p = TerminalProfile::create_default();
    p.name = "Test Profile";
    p.font_size = 18;
    manager.save_profile(p);
    
    // Read
    auto retrieved = manager.get_profile("Test Profile");
    EXPECT_EQ(retrieved.font_size, 18);
    EXPECT_EQ(retrieved.background_color, "#1e1e1e");
    
    // Update
    p.cursor_blink = false;
    manager.save_profile(p);
    EXPECT_FALSE(manager.get_profile("Test Profile").cursor_blink);
    
    // Delete
    manager.delete_profile("Test Profile");
    auto names = manager.get_profile_names();
    bool found = false;
    for (const auto& n : names) if (n == "Test Profile") found = true;
    EXPECT_FALSE(found);
}

TEST_F(ProfileManagerTest, Persistence) {
    {
        ProfileManager manager;
        auto p = TerminalProfile::create_default();
        p.name = "Persisted";
        p.startup_command = "/bin/zsh";
        manager.save_profile(p);
        manager.set_default_profile("Persisted");
    }
    
    // Reload
    {
        ProfileManager manager; // Should load from disk
        EXPECT_EQ(manager.get_default_profile_name(), "Persisted");
        auto p = manager.get_profile("Persisted");
        EXPECT_EQ(p.startup_command, "/bin/zsh");
    }
}
