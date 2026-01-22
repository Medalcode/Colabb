#include "application/prediction_service.hpp"
#include <iostream>

namespace colabb {
namespace application {

PredictionService::PredictionService(std::unique_ptr<domain::IAIProvider> provider)
    : ai_provider_(std::move(provider))
    , stop_worker_(false) {
    
    worker_thread_ = std::thread(&PredictionService::worker_loop, this);
}

PredictionService::~PredictionService() {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        stop_worker_ = true;
    }
    queue_cv_.notify_one();
    
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

void PredictionService::predict_async(const std::string& query,
                                      const std::string& context,
                                      PredictionCallback callback) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        request_queue_.push({query, context, callback});
    }
    queue_cv_.notify_one();
}

void PredictionService::cancel_pending() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    while (!request_queue_.empty()) {
        request_queue_.pop();
    }
}

void PredictionService::worker_loop() {
    while (true) {
        PredictionRequest request;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] {
                return stop_worker_ || !request_queue_.empty();
            });
            
            if (stop_worker_ && request_queue_.empty()) {
                break;
            }
            
            if (!request_queue_.empty()) {
                request = std::move(request_queue_.front());
                request_queue_.pop();
            } else {
                continue;
            }
        }
        
        // Process request
        try {
            auto suggestion = ai_provider_->predict(request.query, request.context);
            request.callback(suggestion);
        } catch (const std::exception& e) {
            std::cerr << "Prediction error: " << e.what() << std::endl;
            request.callback(std::nullopt);
        }
    }
}

} // namespace application
} // namespace colabb
