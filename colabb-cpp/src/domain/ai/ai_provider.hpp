#ifndef COLABB_AI_PROVIDER_HPP
#define COLABB_AI_PROVIDER_HPP

#include "domain/models/suggestion.hpp"
#include "infrastructure/http/http_client.hpp"
#include <string>
#include <memory>
#include <optional>

namespace colabb {
namespace domain {

class IAIProvider {
public:
    virtual ~IAIProvider() = default;
    
    virtual std::optional<Suggestion> predict(const std::string& prompt,
                                               const std::string& context = "") = 0;
    virtual bool validate_connection() = 0;
};

class GroqProvider : public IAIProvider {
public:
    explicit GroqProvider(const std::string& api_key);
    
    std::optional<Suggestion> predict(const std::string& prompt,
                                      const std::string& context = "") override;
    bool validate_connection() override;
    
private:
    std::string api_key_;
    std::unique_ptr<infrastructure::HttpClient> http_client_;
    
    std::string sanitize_response(const std::string& raw);
    std::string build_prompt(const std::string& query, const std::string& context);
};

class OpenAIProvider : public IAIProvider {
public:
    explicit OpenAIProvider(const std::string& api_key);
    
    std::optional<Suggestion> predict(const std::string& prompt,
                                      const std::string& context = "") override;
    bool validate_connection() override;
    
private:
    std::string api_key_;
    std::unique_ptr<infrastructure::HttpClient> http_client_;
    
    std::string sanitize_response(const std::string& raw);
};

} // namespace domain
} // namespace colabb

#endif // COLABB_AI_PROVIDER_HPP
