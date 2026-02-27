# Agent (Agente) — Diseño y Operaciones de Colabb

Este documento define la arquitectura, responsabilidades y puntos de extensión del "agent" en Colabb. El objetivo es mantener un componente versátil, escalable y seguro.

## 1. El Agente Generalista (Recomendado)

Para evitar la fragmentación en múltiples archivos `.md` o agentes específicos (p. ej., un "Agente de Git" y un "Agente de Terminal"), Colabb utiliza un **Agente Generalista**.

- **Versatilidad**: En lugar de roles fijos, el agente adapta su comportamiento basándose en el contexto recibido (`Prompt` y `Context`).
- **Contexto Dinámico**: Utiliza `ContextService` para inyectar información relevante (salida de comandos, errores, estado del repo) en el prompt del sistema.
- **Densidad**: Preferimos un único agente potente que gestione múltiples tareas mediante parámetros, en lugar de una explosión de agentes especializados.

## 2. Arquitectura y Componentes Clave

El flujo de ejecución es: **UI** → `PredictionService` → `IAIProvider` → **Servicio Externo** → `SuggestionCache` → **UI**.

- **PredictionService**: Orquestador que gestiona la cola de solicitudes asíncronas ([prediction_service.hpp](colabb-cpp/src/application/prediction_service.hpp)).
- **IAIProvider**: Interfaz para los proveedores de IA (OpenAI, Groq, o plugins dinámicos) ([ai_provider.hpp](colabb-cpp/src/domain/ai/ai_provider.hpp)).
- **SuggestionCache**: Cache LRU con TTL para evitar llamadas redundantes ([suggestion_cache.hpp](colabb-cpp/src/application/suggestion_cache.hpp)).
- **PluginLoader**: Carga de proveedores externos como librerías compartidas ([plugin_loader.hpp](colabb-cpp/src/infrastructure/plugins/plugin_loader.hpp)).
- **ConfigManager / ProfileManager**: Gestión de secretos (libsecret) y perfiles de usuario.

## 3. Modelo de Hilos y Concurrencia

- **Worker-Callback**: Las predicciones se ejecutan en un hilo secundario; los resultados se devuelven vía callback.
- **Seguridad**: `SuggestionCache` es actualmente *thread-unsafe*. Debe protegerse con mutex si se accede desde múltiples workers o marshallear accesos al hilo principal.
- **Timeouts**: Cada solicitud debe tener un timeout externo (watchdog) de ~500ms para modelos locales y < 2s para remotos.

## 4. Extensibilidad vía Plugins

Punto de extensión primario: `IAIProvider`. Para añadir nuevas capacidades sin recompilar:
1. Implementar la interfaz en una `SHARED` library.
2. Exponer símbolos C: `create_provider`, `destroy_provider`, `plugin_api_version`.
3. El `PluginLoader` se encargará del ciclo de vida (ABI C estable).

---
**Estado**: CONSOLIDADO | **Última actualización**: 27 de febrero de 2026

