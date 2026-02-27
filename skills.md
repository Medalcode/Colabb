# Skills — Habilidades y Proveedores

Este archivo es la puerta de entrada a las habilidades (IA Providers) de Colabb.

## 1. Super-Skill Paramétrica (`GenericAiProvider`)
Hemos consolidado la lógica de proveedores individuales en una única **Super-Skill**.

- **Atributos**: `api_key`, `endpoint`, `model_name`.
- **Reutilización**: En lugar de crear un archivo por modelo, se configura una instancia de este proveedor genérico.

## 2. Archivos Obsoletos (Detección de Huérfanos)
Tras la consolidación, los siguientes archivos se consideran candidatos a eliminación:
- `colabb-cpp/src/domain/ai/groq_provider.cpp` (reemplazado por Super-Skill).
- `colabb-cpp/src/domain/ai/openai_provider.cpp` (reemplazado por Super-Skill).

---
*Para el contrato técnico y ABI de plugins, ver [colabb-cpp/docs/skills.md](colabb-cpp/docs/skills.md)*
