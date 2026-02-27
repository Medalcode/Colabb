#ifndef COLABB_AI_PROVIDER_HPP
#define COLABB_AI_PROVIDER_HPP

#include "domain/models/suggestion.hpp"
#include "infrastructure/http/http_client.hpp"
#include <string>
#include <memory>
#include <optional>

namespace colabb {
namespace domain {

/**
 * @brief Interfaz base para proveedores de IA.
 */
class IAIProvider {
public:
    virtual ~IAIProvider() = default;
    
    virtual std::optional<Suggestion> predict(const std::string& prompt,
                                               const std::string& context = "") = 0;
    virtual bool validate_connection() = 0;
};

/**
 * @brief Super-Skill: Proveedor genérico para cualquier servicio compatible con HTTP/JSON (tipo OpenAI/Groq).
 */
class GenericHttpAiProvider : public IAIProvider {
public:
    struct Config {
        std::string api_key;
        std::string endpoint_url;
        std::string model;
        std::string system_prompt;
    };

    explicit GenericHttpAiProvider(Config config);
    
    std::optional<Suggestion> predict(const std::string& prompt,
                                      const std::string& context = "") override;
    bool validate_connection() override;
    
private:
    Config config_;
    std::unique_ptr<infrastructure::HttpClient> http_client_;
    
    std::string sanitize_response(const std::string& raw);
    std::string build_full_prompt(const std::string& query, const std::string& context);
};

} // namespace domain
} // namespace colabb

#endif // COLABB_AI_PROVIDER_HPP
