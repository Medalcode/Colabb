// Integration test: compila y carga el plugin de ejemplo y ejecuta PredictionService
#include <gtest/gtest.h>
#include "colabb/infrastructure/plugins/plugin_loader.hpp"
#include "colabb/application/prediction_service.hpp"

using namespace colabb;

TEST(PluginLoaderIntegration, LoadAndPredict) {
    infrastructure::PluginLoader loader;
    // Ruta relativa al ejemplo compilado por CI/local
    std::string path = "./libexample_provider.so"; // Nota: CI adaptará a .dll/.dylib
    std::string cfg = "{}";
    auto provider = loader.loadProvider(path, cfg);
    ASSERT_NE(provider, nullptr);

    application::PredictionService svc(std::move(provider), 5);
    bool called = false;
    svc.predict_async("hola", "", [&](std::optional<application::PredictionResult> r){
        called = true;
        ASSERT_TRUE(r.has_value());
    });

    // Esperar un tiempo razonable en tests (la implementación puede exponer sync helpers)
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ASSERT_TRUE(called);
}
