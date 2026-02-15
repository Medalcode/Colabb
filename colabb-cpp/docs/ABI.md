# ABI y contrato mínimo para plugins (skills)

Este archivo formaliza el contrato C mínimo exigido a las librerías de proveedor dinámico (plugins). Debe mantenerse retrocompatible y versionado.

API pública mínima (C ABI):

```c
// Devuelve la versión del ABI aceptada por el plugin (ej: "1.0")
extern "C" const char* plugin_api_version();

// Crea e inicializa un provider. `config_json` es un JSON con configuración.
extern "C" domain::IAIProvider* create_provider(const char* config_json);

// Destruye el provider creado por `create_provider`.
extern "C" void destroy_provider(domain::IAIProvider* p);
```

Reglas de ownership:

- El host (Colabb) llama a `create_provider`, usa el puntero devuelto y finalmente llama a `destroy_provider` en el mismo proceso.
- Los plugins no deben asumir compatibilidad binaria con implementaciones particulares de la STL; por eso la configuración se pasa por JSON y los datos se devuelven por objetos simples.

Versionado:

- Incrementar `abi_version` cuando se modifiquen las funciones públicas o cambien las garantías de ownership.

Seguridad:

- El host debe verificar la firma/hash del binario y/o manifiesto antes de cargarlo.
- Para plugins no verificados, ejecutar en proceso aislado y comunicar por IPC.
