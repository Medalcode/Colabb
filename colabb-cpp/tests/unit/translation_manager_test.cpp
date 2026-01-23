#include <gtest/gtest.h>
#include "infrastructure/i18n/translation_manager.hpp"

using namespace colabb::infrastructure;

class TranslationManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset to known state if possible, though singleton makes it hard.
        // We will just set language explicitly in each test.
    }
};

TEST_F(TranslationManagerTest, DefaultLanguageFallback) {
    auto& tm = TranslationManager::instance();
    // Assuming default is loaded on init, but let's ensure we can get a key
    // We can't guarantee state across tests due to Singleton, so be careful.
    
    // Check key that definitely exists in both
    std::string key = "menu.file";
    
    tm.load_language("en");
    EXPECT_EQ(tm.get(key), "File");
    
    tm.load_language("es");
    EXPECT_EQ(tm.get(key), "Archivo");
}

TEST_F(TranslationManagerTest, MissingKeyReturnsKey) {
    auto& tm = TranslationManager::instance();
    std::string missingKey = "non.existent.key";
    
    EXPECT_EQ(tm.get(missingKey), missingKey);
}

TEST_F(TranslationManagerTest, SwitchLanguages) {
    auto& tm = TranslationManager::instance();
    
    tm.load_language("es");
    EXPECT_EQ(tm.get("tab.new"), "Nueva pestaña");
    
    tm.load_language("en");
    EXPECT_EQ(tm.get("tab.new"), "New Tab");
}
