#ifndef COLABB_PREDICTION_SERVICE_HPP
#define COLABB_PREDICTION_SERVICE_HPP

#include "domain/ai/ai_provider.hpp"
#include "domain/models/suggestion.hpp"
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace colabb {
namespace application {

class PredictionService {
public:
    using PredictionCallback = std::function<void(std::optional<domain::Suggestion>)>;
    
    explicit PredictionService(std::unique_ptr<domain::IAIProvider> provider,
                               size_t max_queue_size = 10);
    ~PredictionService();
    
    // Async prediction
    void predict_async(const std::string& query,
                      const std::string& context,
                      PredictionCallback callback);
    
    // Cancel pending predictions
    void cancel_pending();
    
private:
    struct PredictionRequest {
        std::string query;
        std::string context;
        PredictionCallback callback;
    };
    
    std::unique_ptr<domain::IAIProvider> ai_provider_;
    std::thread worker_thread_;
    std::queue<PredictionRequest> request_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    bool stop_worker_;
    size_t max_queue_size_;
    
    void worker_loop();
};

} // namespace application
} // namespace colabb

#endif // COLABB_PREDICTION_SERVICE_HPP
