// Skill skeleton - ejemplo ampliado (ABI 1.0)
#include <string>
#include <optional>
#include "domain/models/suggestion.hpp"
#include "domain/ai/ai_provider.hpp"

using namespace colabb::domain;

class ExampleProvider : public ai::IAIProvider {
public:
    ExampleProvider(const std::string& cfg) {}
    ~ExampleProvider() override = default;

    std::optional<models::Suggestion> predict(const std::string& prompt, const std::string& context = "") override {
        models::Suggestion s;
        s.text = std::string("Respuesta ejemplo para: ") + prompt;
        return s;
    }

    bool validate_connection() override { return true; }
};

extern "C" const char* plugin_api_version() {
    return "1.0";
}

extern "C" colabb::domain::ai::IAIProvider* create_provider(const char* config_json) {
    return new ExampleProvider(config_json ? config_json : "");
}

extern "C" void destroy_provider(colabb::domain::ai::IAIProvider* p) {
    delete p;
}
