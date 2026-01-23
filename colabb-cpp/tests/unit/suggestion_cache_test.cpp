#include <gtest/gtest.h>
#include "application/suggestion_cache.hpp"
#include <thread>
#include <chrono>

using namespace colabb::application;
using namespace colabb::domain;

class SuggestionCacheTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    SuggestionCache cache_;
};

TEST_F(SuggestionCacheTest, PutAndGet) {
    Suggestion s{"ls -la", "List files"};
    cache_.put("list files", s);
    
    auto result = cache_.get("list files");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->command, "ls -la");
    EXPECT_EQ(result->explanation, "List files");
}

TEST_F(SuggestionCacheTest, Normalization) {
    Suggestion s{"pwd", "Print working directory"};
    cache_.put("  List   Files  ", s);
    
    // Should be able to retrieve with normalized key
    auto result = cache_.get("list files");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->command, "pwd");
}

TEST_F(SuggestionCacheTest, CaseInsensitivity) {
    Suggestion s{"git status", "Check status"};
    cache_.put("GIT STATUS", s);
    
    auto result = cache_.get("git status");
    ASSERT_TRUE(result.has_value());
}

TEST_F(SuggestionCacheTest, Miss) {
    auto result = cache_.get("non checkexistent");
    ASSERT_FALSE(result.has_value());
}

TEST_F(SuggestionCacheTest, TTL) {
    // Note: This test implies waiting, which is acceptable for unit tests but generally we'd want to mock time.
    // Since SuggestionCache uses std::chrono::system_clock, we can't easily mock it without refactoring.
    
    // However, we can test that it DOESN'T expire immediately.
    Suggestion s{"cmd", "desc"};
    cache_.put("key", s);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto result = cache_.get("key");
    ASSERT_TRUE(result.has_value());
}

// To test LRU, we need to know the MAX_SIZE. 
// Looking at the header/cpp (which I should have checked closely), it's 100.
// We can fill it up and check eviction.
TEST_F(SuggestionCacheTest, LRUEviction) {
    // Fill the cache
    for (int i = 0; i < 100; ++i) {
        Suggestion s{"cmd" + std::to_string(i), "desc"};
        cache_.put("key" + std::to_string(i), s);
    }
    
    // Add one more
    Suggestion s_new{"new", "new"};
    cache_.put("key_new", s_new);
    
    // The first one (key0) should be evicted (LRU)
    auto result0 = cache_.get("key0");
    ASSERT_FALSE(result0.has_value());
    
    // The new one should be there
    auto result_new = cache_.get("key_new");
    ASSERT_TRUE(result_new.has_value());
    
    // The second one (key1) should still be there
    auto result1 = cache_.get("key1");
    ASSERT_TRUE(result1.has_value());
}
