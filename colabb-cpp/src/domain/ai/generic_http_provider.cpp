#include "domain/ai/ai_provider.hpp"
#include <nlohmann/json.hpp>
#include <regex>
#include <iostream>

using json = nlohmann::json;

namespace colabb {
namespace domain {

GenericHttpAiProvider::GenericHttpAiProvider(Config config)
    : config_(std::move(config))
    , http_client_(std::make_unique<infrastructure::HttpClient>()) {
}

std::string GenericHttpAiProvider::build_full_prompt(const std::string& query, const std::string& context) {
    if (context.empty()) {
        return query;
    }
    return "Context:\n" + context + "\n\nQuery: " + query;
}

std::optional<Suggestion> GenericHttpAiProvider::predict(const std::string& prompt,
                                                         const std::string& context) {
    std::string full_prompt = build_full_prompt(prompt, context);
    
    json request_body = {
        {"model", config_.model},
        {"messages", {
            {{"role", "system"}, {"content", config_.system_prompt}},
            {{"role", "user"}, {"content", full_prompt}}
        }},
        {"temperature", 0.1},
        {"max_tokens", 100}
    };
    
    std::map<std::string, std::string> headers = {
        {"Authorization", "Bearer " + config_.api_key},
        {"Content-Type", "application/json"}
    };
    
    auto response = http_client_->post(config_.endpoint_url, request_body.dump(), headers);
    
    if (!response.is_success()) {
        std::cerr << "AI Provider error (" << config_.model << "): " << response.status_code << std::endl;
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
        std::cerr << "Failed to parse AI response: " << e.what() << std::endl;
        return std::nullopt;
    }
}

bool GenericHttpAiProvider::validate_connection() {
    // Para validación rápida, pedimos un solo token.
    json request_body = {
        {"model", config_.model},
        {"messages", {{{"role", "user"}, {"content", "ping"}}}},
        {"max_tokens", 1}
    };
    
    std::map<std::string, std::string> headers = {
        {"Authorization", "Bearer " + config_.api_key},
        {"Content-Type", "application/json"}
    };
    
    auto response = http_client_->post(config_.endpoint_url, request_body.dump(), headers);
    return response.is_success();
}

std::string GenericHttpAiProvider::sanitize_response(const std::string& raw) {
    std::string result = raw;
    
    // Limpieza de bloques de código markdown
    result = std::regex_replace(result, std::regex("```[a-z]*"), "");
    result = std::regex_replace(result, std::regex("```"), "");
    result = std::regex_replace(result, std::regex("`"), "");
    
    // Eliminar espacios en blanco al inicio y al final
    const auto first = result.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        return "";
    }
    result.erase(0, first);
    const auto last = result.find_last_not_of(" \t\n\r");
    if (last != std::string::npos) {
        result.erase(last + 1);
    }

    // Mantener solo la primera línea para evitar comandos múltiples accidentales
    auto newline_pos = result.find('\n');
    if (newline_pos != std::string::npos) {
        result = result.substr(0, newline_pos);
    }
    
    // Quitar prefijo de shell común si existe
    if (result.rfind("$ ", 0) == 0) {
        result = result.substr(2);
    }
    
    return result;
}

} // namespace domain
} // namespace colabb
