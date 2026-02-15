# Agent — Diseño y Operaciones

Última actualización: 15 de febrero de 2026

Este documentodescribe el diseño, responsabilidades y requisitos operativos del "agent" dentro de Colabb. El objetivo es definir un componente escalable, seguro y testeable que orqueste las predicciones de IA, la gestión de caché y el ciclo de vida de los proveedores (providers).

## Resumen y propósito

El `agent` es la capa que coordina:

- Recepción de peticiones desde la UI (`MainWindow`/`SearchBar`).
- Encolado y ejecución asíncrona en `PredictionService`.
- Conversación con un `IAIProvider` (local o plugin) para obtener sugerencias.
- Almacenamiento y reutilización en `SuggestionCache`.
- Gestión de credenciales y perfiles a través de `ConfigManager`/`ProfileManager`.

Flujo de alto nivel: UI → `PredictionService` → `IAIProvider` → `HttpClient`/plugin → `SuggestionCache` → UI.

Referencias: `colabb-cpp/src/application/prediction_service.hpp`, `colabb-cpp/src/domain/ai/ai_provider.hpp`, `colabb-cpp/src/application/suggestion_cache.hpp`.

## Componentes clave

- `PredictionService` — orquesta la cola de predicciones, trabajadores y callbacks. Ver `prediction_service.hpp`.
- `SuggestionCache` — LRU + TTL para evitar solicitudes redundantes. Ver `suggestion_cache.hpp`.
- `PluginLoader` — responsable de cargar providers externos. Ver `infrastructure/plugins/plugin_loader.hpp`.
- `ConfigManager` / `ProfileManager` — almacenan API keys (libsecret) y perfiles de usuario.

## Requisitos de concurrencia y modelo de hilos

- `PredictionService` ejecuta predicciones en un worker thread; las APIs públicas son no bloqueantes y usan callbacks.
- Cualquier acceso compartido a `SuggestionCache` o estructuras mutables debe ser sincronizado. Si se permite acceso concurrente desde plugins, la cache debe protegerse con mutexes.

Recomendación: documentar y/o imponer que solo `PredictionService` interactúe con la cache, o bien convertir la cache en thread-safe.

## Seguridad y políticas para plugins

- No cargar código arbitrario en producción. Solo se permitirán plugins firmados/verificados por el equipo (firmas o hash verificados).
- Verificar manifiesto del plugin antes de load (metadatos: `name`, `version`, `entry_symbol`, `capabilities`).
- Ejecutar providers no confiables en entornos aislados (procesos separados, seccomp, contenedores) y aplicar límites de recursos y timeouts.

Riesgos: ejecución de código malicioso, fugas de credenciales, incompatibilidad ABI. Ver `infrastructure/plugins/plugin_loader.cpp`.

## Límites, timeouts y SLOs

- Cada llamada `predict` debe tener un timeout externo (watchdog) además del timeout del `HttpClient`.
- `PredictionService` debe exponerse con parámetros configurables: `max_queue_size`, `worker_count`, `per_request_timeout`.

Propuesta inicial de SLOs:
- 95th latency por predicción < 500 ms (modelo local) / < 2s (modelo remoto)
- Reintentos con backoff exponencial para fallos transitorios.

## Pruebas y monitoreo

- Unit tests para `PredictionService`, `SuggestionCache`, `PluginLoader` (ya existen tests para cola y cache).
- Tests de integración E2E que compilan un plugin de ejemplo, lo cargan con `PluginLoader` y ejercen `PredictionService`.
- Métricas sugeridas: latencia por predicción, tasa de fallos, uso de memoria por plugin, tiempo de carga del plugin.

## Operaciones y despliegue

- Build reproducible con `CMakeLists.txt` (ver `colabb-cpp/CMakeLists.txt`).
- Empaquetado: `.deb` script en `scripts/package_deb.sh`.
- CI: compilar en Linux/macOS/Windows, ejecutar tests unitarios e integration tests.

## Checklist de hardening antes de producción

1. Estabilizar y versionar el ABI de plugins (ver `colabb-cpp/docs/ABI.md`).
2. Implementar verificación de firmas/hashes para plugins.
3. Añadir watchdog externo por `predict`.
4. Hacer `SuggestionCache` thread-safe o limitar accesos.
5. Añadir soporte multiplataforma para almacenamiento seguro de claves.
6. Añadir tests E2E y CI que comprueben carga de plugin y comportamiento frente a timeouts.

## Anexos y referencias rápidas

- Firma mínima esperada para plugins: `extern "C" domain::IAIProvider* create_provider(const char* config_json);` y `extern "C" void destroy_provider(domain::IAIProvider* p);` (ver `docs/examples/skill_skeleton.cpp`).
- Archivos mencionados: `colabb-cpp/src/application/prediction_service.hpp`, `colabb-cpp/src/infrastructure/plugins/plugin_loader.cpp`.
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
