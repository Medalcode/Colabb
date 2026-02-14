// Ejemplo mínimo de Skill como shared lib (DRAFT)
// Ruta de ejemplo de includes: ajustar `-I` en CMake según proyecto.
#include <optional>
#include <string>
#include <memory>
#include <ctime>

#include "../../src/domain/ai/ai_provider.hpp"
#include "../../src/domain/models/suggestion.hpp"

using namespace domain;

class SkeletonProvider : public IAIProvider {
public:
    SkeletonProvider(const std::string& config_json) : config_(config_json) {}

    std::optional<Suggestion> predict(const std::string& prompt, const std::string& context) override {
        Suggestion s;
        s.command = "echo \"SUGGESTION: " + prompt + "\"";
        s.explanation = "Respuesta generada por SkeletonProvider";
        s.confidence = 0.5;
        s.timestamp = std::time(nullptr);
        return std::optional<Suggestion>(s);
    }

    bool validate_connection() override {
        return true;
    }

private:
    std::string config_;
};

extern "C" IAIProvider* create_provider(const char* config_json) {
    try {
        return new SkeletonProvider(config_json ? config_json : "{}");
    } catch(...) {
        return nullptr;
    }
}

extern "C" void destroy_provider(IAIProvider* p) {
    delete p;
}
