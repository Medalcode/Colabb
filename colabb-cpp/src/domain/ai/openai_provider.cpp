#include "domain/ai/ai_provider.hpp"
#include <nlohmann/json.hpp>
#include <regex>
#include <iostream>

using json = nlohmann::json;

namespace colabb {
namespace domain {

OpenAIProvider::OpenAIProvider(const std::string& api_key)
    : api_key_(api_key)
    , http_client_(std::make_unique<infrastructure::HttpClient>()) {
}

std::optional<Suggestion> OpenAIProvider::predict(const std::string& prompt,
                                                   const std::string& context) {
    const std::string url = "https://api.openai.com/v1/chat/completions";
    
    std::string full_prompt = context.empty() ? prompt : "Context:\n" + context + "\n\nQuery: " + prompt;
    
    json request_body = {
        {"model", "gpt-3.5-turbo"},
        {"messages", {
            {{"role", "system"}, {"content", "You are a Linux terminal expert. Return ONLY the suggested bash command, without explanations or markdown formatting."}},
            {{"role", "user"}, {"content", full_prompt}}
        }},
        {"temperature", 0.1},
        {"max_tokens", 80}
    };
    
    std::map<std::string, std::string> headers = {
        {"Authorization", "Bearer " + api_key_},
        {"Content-Type", "application/json"}
    };
    
    auto response = http_client_->post(url, request_body.dump(), headers);
    
    if (!response.is_success()) {
        std::cerr << "OpenAI API error: " << response.status_code << std::endl;
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
        std::cerr << "Failed to parse OpenAI response: " << e.what() << std::endl;
        return std::nullopt;
    }
}

bool OpenAIProvider::validate_connection() {
    const std::string url = "https://api.openai.com/v1/models";
    
    std::map<std::string, std::string> headers = {
        {"Authorization", "Bearer " + api_key_}
    };
    
    auto response = http_client_->get(url, headers);
    return response.is_success();
}

std::string OpenAIProvider::sanitize_response(const std::string& raw) {
    std::string result = raw;
    
    // Remove markdown code blocks
    result = std::regex_replace(result, std::regex("```bash"), "");
    result = std::regex_replace(result, std::regex("```"), "");
    result = std::regex_replace(result, std::regex("`"), "");
    
    // Trim whitespace
    const auto first = result.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        return "";
    }
    result.erase(0, first);
    const auto last = result.find_last_not_of(" \t\n\r");
    if (last != std::string::npos) {
        result.erase(last + 1);
    }

    // Keep only first non-empty line to prevent multi-command outputs.
    auto newline_pos = result.find('\n');
    if (newline_pos != std::string::npos) {
        result = result.substr(0, newline_pos);
    }
    if (result.rfind("$ ", 0) == 0) {
        result = result.substr(2);
    }
    
    return result;
}

} // namespace domain
} // namespace colabb
