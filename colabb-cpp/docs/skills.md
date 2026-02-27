# Skills (Habilidades) — Contrato y Super-Skills

Este documento define el contrato para desarrollar "skills" (proveedores de IA) para Colabb, facilitando la creación de componentes reutilizables y parametrizados.

## 1. Super-Skills Paramétricas (Recomendado)

Para evitar crear una skill nueva por cada modelo o servicio (p. ej., `OpenAISkill`, `GroqSkill`, `LlamaSkill`), fomentamos el uso de **Super-Skills**.

- **Principio**: Una skill debe ser genérica y recibir parámetros vía configuración.
- **Parametrización**: En lugar de logic hardcoded, se usa un `config_json` con:
    - `api_url`: Endpoint del servicio.
    - `api_key`: Llave de autenticación.
    - `model`: Identificador del modelo (p. ej., `gpt-4`, `llama-3.1`).
- **Beneficio**: Reduce el código redundante en un 80% al compartir la lógica de transporte HTTP y parseo JSON.

## 2. Contrato de Skill (`IAIProvider`)

Cualquier skill debe implementar la interfaz `IAIProvider` ([ai_provider.hpp](colabb-cpp/src/domain/ai/ai_provider.hpp)):

- `predict(prompt, context)`: Devuelve una sugerencia opcional.
- `validate_connection()`: Comprueba credenciales y conectividad.

## 3. Implementación y Plugins Dinámicos

Los skills pueden compilarse estáticamente o cargarse dinámicamente como plugins:

### ABI C para Plugins:
- `extern "C" const char* plugin_api_version();`
- `extern "C" domain::IAIProvider* create_provider(const char* config_json);`
- `extern "C" void destroy_provider(domain::IAIProvider* p);`

### Manifest (`skill_manifest.json`):
```json
{
  "name": "generic_http_provider",
  "version": "1.0.0",
  "capabilities": ["suggestion", "explanation"],
  "params": ["api_url", "api_key", "model"]
}
```

## 4. Mejores Prácticas y Seguridad

1. **Reutilización**: Antes de crear un nuevo `.cpp`, verifica si la Super-Skill genérica puede manejar el nuevo servicio mediante un cambio de URL/Template.
2. **Aislamiento**: Los plugins deben ejecutarse con privilegios mínimos y timeouts estrictos.
3. **Secretos**: Nunca hardcodear llaves; usar `ConfigManager` para recuperarlas de `libsecret`.

---
**Estado**: CONSOLIDADO | **Última actualización**: 27 de febrero de 2026

