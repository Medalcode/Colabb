# Agent (Agente) — Colabb

Propósito
- Definir qué es un "agente" dentro de Colabb y describir el flujo de ejecución, responsabilidades y puntos de extensión.

Arquitectura al alto nivel
- Flujo: UI → PredictionService → IAIProvider → HttpClient → Proveedor externo (OpenAI, Groq, etc.)
- Componentes clave:
  - PredictionService: orquestador de solicitudes de predicción y cola de trabajo ([colabb-cpp/src/application/prediction_service.hpp](colabb-cpp/src/application/prediction_service.hpp)).
  - IAIProvider: interfaz de proveedor de IA; implementaciones actuales: OpenAI y Groq ([colabb-cpp/src/domain/ai/ai_provider.hpp](colabb-cpp/src/domain/ai/ai_provider.hpp)).
  - SuggestionCache: cache LRU con TTL para evitar llamadas repetidas ([colabb-cpp/src/application/suggestion_cache.hpp](colabb-cpp/src/application/suggestion_cache.hpp)).
  - ContextService: detección y construcción de prompts basados en proyecto ([colabb-cpp/src/infrastructure/context/context_service.hpp](colabb-cpp/src/infrastructure/context/context_service.hpp)).
  - ConfigManager / ProfileManager: manejo de providers, llaves y perfiles ([colabb-cpp/src/infrastructure/config/config_manager.hpp](colabb-cpp/src/infrastructure/config/config_manager.hpp)).

Modelo de hilos y seguridad
- `PredictionService` procesa predicciones en un worker thread y notifica mediante callbacks; la UI marshalling se hace con `g_idle_add` en GTK (ver [colabb-cpp/src/ui/main_window.cpp](colabb-cpp/src/ui/main_window.cpp)).
- Restricción actual: `SuggestionCache` no está protegido por mutex — asume acceso desde el hilo principal. Si se habilitan accesos concurrentes desde providers/plugins, se debe proteger con mutex o marshallar accesos.
- Recomendación para providers: no realizar operaciones que bloqueen de manera prolongada sin timeouts ni manejo de errores; respetar el modelo de hilos y retornar resultados rápidamente.

Extensión y plugins
- Punto de extensión primario: `IAIProvider` — para añadir nuevas fuentes de predicción (ej. nuevos modelos, servicios privados).
- Estado actual: las implementaciones se compilan estáticamente dentro del binario. Para soportar plugins dinámicos se recomienda introducir un pequeño `PluginLoader` con ABI C simple (se describe en `skills.md`).
 - Estado actual: las implementaciones se compilan estáticamente dentro del binario. Para soportar plugins dinámicos se recomienda introducir un pequeño `PluginLoader` con ABI C simple (se describe en `skills.md`).
 - Implementación inicial: se añadió `PluginLoader` en `src/infrastructure/plugins/plugin_loader.hpp` y `src/infrastructure/plugins/plugin_loader.cpp`. Este loader carga `create_provider`/`destroy_provider` desde shared libs y devuelve una `std::unique_ptr<domain::IAIProvider>` con deleter adecuado.

Despliegue y verificación
- Para compilar y ejecutar tests del proyecto:

```bash
cmake -S colabb-cpp -B build
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

- Empaquetado: ver [colabb-cpp/scripts/package_deb.sh](colabb-cpp/scripts/package_deb.sh) y [colabb-cpp/snap/snapcraft.yaml](colabb-cpp/snap/snapcraft.yaml).

Notas operativas
- Documentar y versionar cualquier cambio en el contrato de `IAIProvider` (ABI/headers) antes de permitir plugins dinámicos.
- Si se necesita soporte runtime para providers, planear:
  1) definir un ABI C (create/destroy),
  2) implementar `PluginLoader` (dlopen / LoadLibrary),
  3) actualizar `CMakeLists.txt` para construir providers como `SHARED` cuando se desee.

Referencias
- Código: [colabb-cpp/src/application/prediction_service.hpp](colabb-cpp/src/application/prediction_service.hpp), [colabb-cpp/src/domain/ai/ai_provider.hpp](colabb-cpp/src/domain/ai/ai_provider.hpp)
- Tests relevantes: [colabb-cpp/tests/unit/prediction_service_queue_test.cpp](colabb-cpp/tests/unit/prediction_service_queue_test.cpp)

Estado: DRAFT — revisar y ajustar detalles técnicos según feedback.
