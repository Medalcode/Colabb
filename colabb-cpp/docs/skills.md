# Skills — Cómo crear un proveedor/skill para Colabb

Resumen
- Un "skill" en Colabb es un componente que suministra sugerencias/predicciones. A nivel de código, un skill implementa la interfaz `IAIProvider` (ver [colabb-cpp/src/domain/ai/ai_provider.hpp](colabb-cpp/src/domain/ai/ai_provider.hpp)).

Contrato de la interfaz
- Métodos principales esperados por `IAIProvider`:
  - `std::optional<domain::Suggestion> predict(const std::string& prompt, const std::string& context)` — realizar la predicción/sugerencia.
  - `bool validate_connection()` — validar credenciales/conectividad.

Tipos
- `domain::Suggestion` contiene al menos: `command`, `explanation`, `confidence`, `timestamp` ([colabb-cpp/src/domain/models/suggestion.hpp](colabb-cpp/src/domain/models/suggestion.hpp)).

Modelos de extensión
- 1) Implementación estática: añadir una nueva clase que herede de `IAIProvider` y compilar dentro del binario.
- 2) Plugins dinámicos (recomendado para extensibilidad): construir un shared library con una ABI C mínima:

```cpp
extern "C" domain::IAIProvider* create_provider(const char* config_json);
extern "C" void destroy_provider(domain::IAIProvider* p);
```

  - `config_json` puede contener la configuración inicial (API key, timeouts, opciones).
  - El host usará `dlopen` / `LoadLibrary` para cargar y resolver `create_provider`.

Ejemplo mínimo (ver `docs/examples/skill_skeleton.cpp`)
- Implementa una clase que herede de `domain::IAIProvider`, devuelve un `Suggestion` simple y define `create_provider`/`destroy_provider`.

Manifest y metadata
- Se recomienda un `skill_manifest.json` con campos mínimos:

```json
{
  "name": "mi_skill",
  "version": "0.1.0",
  "entry_symbol": "create_provider",
  "thread_model": "worker-callback",
  "capabilities": ["suggestion","explain"]
}
```

Integración con Colabb
- `PredictionService` acepta un `std::unique_ptr<IAIProvider>` en su constructor ([colabb-cpp/src/application/prediction_service.hpp](colabb-cpp/src/application/prediction_service.hpp)). Para integración manual, crear la instancia y pasarla al servicio.
- Guardar credenciales mediante `ConfigManager` y libsecret (ver [colabb-cpp/src/infrastructure/config/config_manager.hpp](colabb-cpp/src/infrastructure/config/config_manager.hpp)).

Empaquetado y CMake
- Ejemplo de snippet para compilar un provider como `SHARED` se encuentra en `docs/examples/CMakeSnippet.txt`.

Pruebas
- Añadir tests unitarios que sustituyan el provider por un "fake" para validar el comportamiento del `PredictionService` y del cache. Ver tests existentes en [colabb-cpp/tests/unit/](colabb-cpp/tests/unit/).

Seguridad y límites
- No incluir claves en repositorios. Usar `ConfigManager` y libsecret para almacenamiento seguro.
- Implementar timeouts y límites de reintento en llamadas externas.

Estado: DRAFT — ver `docs/examples/` para el skeleton y manifest de ejemplo.
