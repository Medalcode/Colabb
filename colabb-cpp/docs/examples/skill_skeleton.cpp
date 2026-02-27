// Skill Skeleton — Ejemplo de Proveedor para Colabb (ABI 1.0)
// Este archivo sirve de base para crear plugins dinámicos.

#include <string>
#include <optional>
#include <ctime>
#include <memory>

#include "domain/ai/ai_provider.hpp"
#include "domain/models/suggestion.hpp"

using namespace colabb::domain;

class ExampleSkeletonProvider : public IAIProvider {
public:
    ExampleSkeletonProvider(const std::string& config_json) : config_(config_json) {}
    ~ExampleSkeletonProvider() override = default;

    std::optional<Suggestion> predict(const std::string& prompt, 
                                       const std::string& context = "") override {
        Suggestion s;
        s.command = "echo \"Respuesta para: " + prompt + "\"";
        s.explanation = "Generado por ExampleSkeletonProvider (ABI 1.0)";
        s.confidence = 0.8f;
        s.timestamp = std::time(nullptr);
        return s;
    }

    bool validate_connection() override {
        return true; 
    }

private:
    std::string config_;
};

// --- Símbolos ABI C ---

extern "C" const char* plugin_api_version() {
    return "1.0";
}

extern "C" IAIProvider* create_provider(const char* config_json) {
    return new ExampleSkeletonProvider(config_json ? config_json : "{}");
}

extern "C" void destroy_provider(IAIProvider* p) {
    delete p;
}
