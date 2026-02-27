# Agentes — Definiciones y Roles de Colabb

Este archivo centraliza la definición de los agentes de IA en el proyecto.

## 1. Agente Generalista (Estándar)
En lugar de fragmentar la lógica en múltiples agentes especializados, utilizamos un único **Agente Generalista**.

- **Rol**: Experto en Terminal Linux y Asistente de Desarrollo.
- **Contexto**: Se alimenta dinámicamente de la salida de la terminal y los archivos del proyecto.
- **Tareas**: 
  - Sugerencia de comandos.
  - Explicación de errores.
  - Generación de scripts.
- **Parametrización**: El comportamiento se ajusta mediante el `system_prompt` enviado al proveedor.

## 2. Consolidación de Roles
Se han fusionado los siguientes roles hipotéticos en el Agente Generalista:
- **TerminalExpert**: Absorbido (ahora es el comportamiento por defecto).
- **GitAssistant**: Absorbido (se maneja inyectando el contexto de `git status`).
- **ErrorHandler**: Absorbido (se activa al detectar códigos de salida != 0).

---
*Para más detalles técnicos, ver [colabb-cpp/docs/agent.md](colabb-cpp/docs/agent.md)*
