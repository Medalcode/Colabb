#include "domain/ai/ai_provider.hpp"
#include <nlohmann/json.hpp>
#include <regex>
#include <iostream>

using json = nlohmann::json;

namespace colabb {
namespace domain {

// ============================================================================
// GroqProvider Implementation
// ============================================================================

GroqProvider::GroqProvider(const std::string& api_key)
    : api_key_(api_key)
    , http_client_(std::make_unique<infrastructure::HttpClient>()) {
}

std::string GroqProvider::build_prompt(const std::string& query, const std::string& context) {
    if (context.empty()) {
        return query;
    }
    return "Context:\n" + context + "\n\nQuery: " + query;
}

std::optional<Suggestion> GroqProvider::predict(const std::string& prompt,
                                                 const std::string& context) {
    const std::string url = "https://api.groq.com/openai/v1/chat/completions";
    
    std::string full_prompt = build_prompt(prompt, context);
    
    json request_body = {
        {"model", "llama-3.1-8b-instant"},
        {"messages", {
            {{"role", "system"}, {"content", "Eres un experto en terminal Linux. Devuelve SOLO el comando bash sugerido, sin explicaciones ni formato markdown."}},
            {{"role", "user"}, {"content", full_prompt}}
        }},
        {"max_tokens", 100}
    };
    
    std::map<std::string, std::string> headers = {
        {"Authorization", "Bearer " + api_key_},
        {"Content-Type", "application/json"}
    };
    
    auto response = http_client_->post(url, request_body.dump(), headers);
    
    if (!response.is_success()) {
        std::cerr << "Groq API error: " << response.status_code << std::endl;
        return std::nullopt;
    }
    
    try {
        json response_json = json::parse(response.body);
        std::string content = response_json["choices"][0]["message"]["content"];
        std::string sanitized = sanitize_response(content);
        
        if (sanitized.empty()) {
            return std::nullopt;
        }
        
        return Suggestion(sanitized);
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse Groq response: " << e.what() << std::endl;
        return std::nullopt;
    }
}

bool GroqProvider::validate_connection() {
    const std::string url = "https://api.groq.com/openai/v1/chat/completions";
    
    json request_body = {
        {"model", "llama-3.1-8b-instant"},
        {"messages", {{{"role", "user"}, {"content", "test"}}}},
        {"max_tokens", 1}
    };
    
    std::map<std::string, std::string> headers = {
        {"Authorization", "Bearer " + api_key_},
        {"Content-Type", "application/json"}
    };
    
    auto response = http_client_->post(url, request_body.dump(), headers);
    return response.is_success();
}

std::string GroqProvider::sanitize_response(const std::string& raw) {
    std::string result = raw;
    
    // Remove markdown code blocks
    result = std::regex_replace(result, std::regex("```bash"), "");
    result = std::regex_replace(result, std::regex("```"), "");
    result = std::regex_replace(result, std::regex("`"), "");
    
    // Trim whitespace
    result.erase(0, result.find_first_not_of(" \t\n\r"));
    result.erase(result.find_last_not_of(" \t\n\r") + 1);
    
    return result;
}

} // namespace domain
} // namespace colabb
