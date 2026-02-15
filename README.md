# Colabb Terminal

Una terminal moderna para Linux potenciada con Inteligencia Artificial, escrita en C++ para máximo rendimiento y escalabilidad.

## 🚀 Características

- **Terminal Real**: Basada en VTE (GTK3), soporta todos tus comandos habituales
- **Asistencia por IA**: Integración con Groq (Llama 3.1) y OpenAI
- **Sistema Totem (`?`)**: Escribe `?` seguido de tu consulta para invocar a la IA
- **Conciencia de Contexto**: La IA lee errores y salidas previas para sugerencias inteligentes
- **Pestañas y Perfiles**: Trabaja en múltiples contextos con configuraciones visuales personalizadas.
- **Internacionalización**: Disponible en Español e Inglés.
- **Autocompletado Híbrido**: Usa `Tab` tanto para autocompletado de shell como para sugerencias de IA.

## 🛠️ Requisitos

### Dependencias del Sistema (Debian/Ubuntu)

```bash
sudo apt install \
    build-essential \
    cmake \
    pkg-config \
    libgtk-3-dev \
    libvte-2.91-dev \
    libcurl4-openssl-dev \
    libjsoncpp-dev \
    libsecret-1-dev
```

## 📦 Compilación e Instalación

```bash
cd colabb-cpp
# Opcional: Generar paquete .deb
./scripts/package_deb.sh
sudo apt install ./colabb_1.0.0_amd64.deb

# O compilar manualmente
mkdir build && cd build
cmake ..
make -j$(nproc)
./colabb
```

## ▶️ Uso

1. **Configuración Inicial**:
   - Ve a `Menú > Preferencias IA`
   - Selecciona tu proveedor (Groq/OpenAI) e ingresa tu API Key.

2. **Asistencia IA Inteligente**:
   - **Preguntar**: Escribe `?` seguido de tu consulta (ej: `? deszipar archivo`).
   - **Explicar Error**: Si un comando falla, presiona `Ctrl + Alt + E` para que la IA analice la salida.
   - **Autocompletado**: Si ves una sugerencia, presiona `Tab` o `Ctrl + Space` para insertarla.

3. **Gestión de Pestañas**:
   - `Ctrl + Shift + T`: Nueva pestaña
   - `Ctrl + Shift + W`: Cerrar pestaña
   - `Ctrl + PageUp / PageDown`: Navegar entre pestañas

4. **Perfiles**:
   - Ve a `Menú > Perfiles` para personalizar fuentes, colores y comandos de inicio.
   - Los cambios se aplican inmediatamente a todas las pestañas abiertas.

## 🤝 Contribución

Si encuentras bugs o tienes ideas, ¡abre un issue o PR!

## 📚 Docs y extensibilidad (nuevas adiciones)

Se han añadido documentos y ejemplos para facilitar la creación de proveedores (skills) y la integración de plugins dinámicos:

- `colabb-cpp/docs/agent.md` — descripción del modelo de agentes, threading y puntos de extensión.
- `colabb-cpp/docs/skills.md` — guía para crear `IAIProvider` y ejemplo de ABI para plugins (`create_provider`/`destroy_provider`).
- `colabb-cpp/docs/examples/` — ejemplo `skill_skeleton.cpp`, `skill_manifest.json` y un `CMake` snippet para compilar providers como `SHARED`.

Además se añadió una implementación inicial de `PluginLoader` en `src/infrastructure/plugins/` y se actualizó `CMakeLists.txt` para enlazar con `dl` en sistemas UNIX. Esto permite cargar proveedores implementados como shared libraries (ver `skills.md` para el contrato ABI y precauciones de seguridad).

## 📄 Licencia

MIT

## Cambios recientes (resumen de la sesión de 2026-01-27)

- `SuggestionCache` reimplementado como LRU para evitar O(n) en expulsión y mejorar rendimiento.
- `PredictionService` ahora acepta `max_queue_size` y rechaza solicitudes cuando la cola está llena (callback con `nullopt`).
- Se añadieron tests unitarios (incluyendo `prediction_service_queue_test.cpp`) y la suite de tests local pasó completamente.

Ver `Bitacora.md` en la raíz para más detalles y próximos pasos.

## Última actualización

Actualizado el 15 de febrero de 2026: se refinó la descripción del proyecto y se añadieron referencias a la carpeta `colabb-cpp/docs` para facilitar la creación de proveedores y plugins.
