// Skill skeleton - ejemplo ampliado (ABI 1.0)
#include <string>
#include <optional>
#include "domain/models/suggestion.hpp"
#include "domain/ai/ai_provider.hpp"

using namespace colabb::domain;

class ExampleProvider : public IAIProvider {
public:
    ExampleProvider(const std::string& cfg) {}
    ~ExampleProvider() override = default;

    std::optional<Suggestion> predict(const std::string& prompt, const std::string& context = "") override {
        Suggestion s;
        s.command = std::string("Respuesta ejemplo para: ") + prompt;
        s.explanation = "(plugin ejemplo)";
        s.confidence = 0.5f;
        return s;
    }

    bool validate_connection() override { return true; }
};

extern "C" const char* plugin_api_version() {
    return "1.0";
}

extern "C" colabb::domain::IAIProvider* create_provider(const char* config_json) {
    return new ExampleProvider(config_json ? config_json : "");
}

extern "C" void destroy_provider(colabb::domain::IAIProvider* p) {
    delete p;
}
