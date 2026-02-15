# Skills — Contrato, Implementación y Buenas Prácticas

Última actualización: 15 de febrero de 2026

Este documento define el contrato para desarrollar "skills" (providers) para Colabb, incluye manifest, ejemplos de implementación (estáticos y dinámicos), requisitos de seguridad, y pruebas recomendadas.

## ¿Qué es un skill?

Un skill es una implementación de la interfaz `IAIProvider` que puede residir internamente en el binario o como una librería dinámica cargada en tiempo de ejecución. Debe exponer comportamiento predecible para `predict()` y cumplir los requisitos de seguridad y compatibilidad ABI.

Referencia: `colabb-cpp/src/domain/ai/ai_provider.hpp`.

## Contrato y ABI recomendados

Recomendación: definir un ABI en C estable y versionado. Mínimo recomendado:

- `extern "C" const char* plugin_api_version();` — devuelve la versión del ABI.
- `extern "C" domain::IAIProvider* create_provider(const char* config_json);`
- `extern "C" void destroy_provider(domain::IAIProvider* p);`

Notas:
- El plugin devuelve un puntero a `IAIProvider` cuyo ciclo de vida está gestionado por `destroy_provider`.
- Documentar ownership: el host es responsable de llamar a `destroy_provider`.
- Evitar pasar objetos C++ ABI-sensitive a través del boundary; usar JSON-strings para configuración.

## Manifest y metadata (skill_manifest.json)

Campos recomendados:

- `name` — identificador legible.
- `version` — semver.
- `abi_version` — versión del ABI que requiere.
- `entry` — símbolo de entrada (p. ej. `create_provider`).
- `capabilities` — lista de capacidades (streaming, batch, secure-storage-needed).
- `signature` — firma o hash para verificación.

Ejemplo: ver `colabb-cpp/docs/examples/skill_manifest.json`.

## Implementación: pasos y CMake

1. Implementa una clase que herede `domain::IAIProvider`.
2. Exponer los símbolos C (`create_provider`/`destroy_provider`/`plugin_api_version`).
3. Compilar como `SHARED` library. Añadir `CMakeLists.txt` snippet.

Snippet de CMake (ver `docs/examples/CMakeSnippet.txt`) muestra cómo crear `.so`/`.dll` multiplataforma.

## Seguridad y sandboxing

- Los plugins deben ejecutarse con privilegios mínimos.
- Requerir firma/verificación antes de carga.
- Recomendado: ejecutar plugins no confiables en proceso separado y comunicarse por IPC (sockets Unix/Windows named pipes) para aislamiento.

## Timeouts y límites

- El host debe imponer `per_request_timeout` y `resource_limits` (memoria, CPU) por plugin.
- Documentar que `predict()` no debe bloquear indefinidamente; si necesita long-running ops debe soportar cancelación.

## Pruebas y CI para skills

- Unit tests que verifiquen comportamiento de `predict()` con entradas conocidas.
- Integration test que compila el plugin, lo carga con `PluginLoader` y ejecuta un escenario `PredictionService`.
- Recomendado: añadir un pipeline CI que compile y ejecute tests en Linux/macOS/Windows.

## Publicación y versionado

- Firmar el `skill_manifest.json` con clave del equipo.
- Mantener compatibilidad ABI; documentar pasos de migración cuando se cambia `abi_version`.

## Ejemplos y plantillas

- `docs/examples/skill_skeleton.cpp` — skeleton básico (actualizado para ABI y multiplataforma).
- `docs/examples/CMakeSnippet.txt` — snippet multiplataforma.
- `docs/examples/skill_manifest.json` — manifest de ejemplo.

---

Si necesitas, puedo generar una plantilla de CI y un plugin ejemplo listo para compilar en Linux/Windows/macOS.
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
