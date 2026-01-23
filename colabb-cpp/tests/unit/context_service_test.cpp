#include <gtest/gtest.h>
#include "infrastructure/context/context_service.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using namespace colabb::infrastructure;

class ContextServiceTest : public ::testing::Test {
protected:
    std::string temp_dir_;

    void SetUp() override {
        temp_dir_ = "/tmp/colabb_test_context_" + std::to_string(std::rand());
        fs::create_directories(temp_dir_);
    }

    void TearDown() override {
        fs::remove_all(temp_dir_);
    }
    
    void create_file(const std::string& name, const std::string& content = "") {
        std::ofstream f(temp_dir_ + "/" + name);
        f << content;
    }
    
    void create_dir(const std::string& name) {
        fs::create_directories(temp_dir_ + "/" + name);
    }
};

TEST_F(ContextServiceTest, EmptyDir) {
    ContextService service;
    auto info = service.detect_context(temp_dir_);
    EXPECT_TRUE(info.languages.empty());
    EXPECT_TRUE(info.build_tools.empty());
    EXPECT_FALSE(info.is_git_repo);
}

TEST_F(ContextServiceTest, DetectCppCMake) {
    create_file("CMakeLists.txt");
    create_file("main.cpp");
    
    ContextService service;
    auto info = service.detect_context(temp_dir_);
    
    ASSERT_EQ(info.languages.size(), 1);
    EXPECT_EQ(info.languages[0], "C++");
    
    ASSERT_EQ(info.build_tools.size(), 1);
    EXPECT_EQ(info.build_tools[0], "CMake");
}

TEST_F(ContextServiceTest, DetectPythonGit) {
    create_file("requirements.txt");
    create_file("script.py");
    create_dir(".git");
    create_file(".git/HEAD", "ref: refs/heads/feature/ai");
    
    ContextService service;
    auto info = service.detect_context(temp_dir_);
    
    bool has_python = false;
    for(const auto& l : info.languages) if(l == "Python") has_python = true;
    EXPECT_TRUE(has_python);
    
    EXPECT_TRUE(info.is_git_repo);
    EXPECT_EQ(info.git_branch, "feature/ai");
}

TEST_F(ContextServiceTest, PromptGeneration) {
    create_file("package.json");
    
    ContextService service;
    std::string prompt = service.get_context_prompt(temp_dir_);
    
    EXPECT_NE(prompt.find("NPM"), std::string::npos);
    EXPECT_NE(prompt.find("JavaScript/TypeScript"), std::string::npos);
}
