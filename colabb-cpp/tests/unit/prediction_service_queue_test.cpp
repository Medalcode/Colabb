#include <gtest/gtest.h>
#include <atomic>
#include <memory>
#include <optional>
#include <chrono>
#include "domain/ai/ai_provider.hpp"
#include "application/prediction_service.hpp"

using namespace colabb;
using namespace colabb::domain;
using namespace colabb::application;

// Fake provider that would normally perform a prediction. For this test it won't be used.
class FakeProvider : public IAIProvider {
public:
    std::optional<Suggestion> predict(const std::string& prompt, const std::string& context = "") override {
        return Suggestion("fake_cmd", "fake", 1.0f);
    }
    bool validate_connection() override { return true; }
};

TEST(PredictionServiceQueueTest, RejectsWhenMaxQueueZero) {
    auto provider = std::make_unique<FakeProvider>();
    // max_queue_size = 0 -> no queueing, should reject immediately
    PredictionService svc(std::move(provider), 0);

    std::atomic<bool> called{false};
    svc.predict_async("q", "c", [&called](std::optional<Suggestion> s) {
        called = true;
        EXPECT_FALSE(s.has_value());
    });

    // Give a tiny moment for the callback to run
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_TRUE(called.load());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
