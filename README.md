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
    libnlohmann-json-dev \
    libsecret-1-dev
```

## 📦 Compilación e Instalación

```bash
cd colabb-cpp
mkdir build && cd build
cmake ..
make -j$(nproc)
./colabb
```

## ▶️ Uso

1. **Configuración Inicial**:
   - Ve a `Menú > Preferencias`
   - Configura tu proveedor de IA (soporta cualquier API compatible con OpenAI/Groq/Local) e ingresa tu API Key. Los secretos se guardan de forma segura en el Ring del Sistema.

2. **Asistencia IA Inteligente**:
   - **Preguntar**: Escribe `?` seguido de tu consulta (ej: `? deszipar archivo`).
   - **Explicar Error**: Si un comando falla, presiona `Ctrl + Alt + E` para que la IA analice la salida.
   - **Autocompletado**: Presiona `Tab` o `Ctrl + Space` para insertar sugerencias.

## 🤝 Contribución

Si encuentras bugs o tienes ideas, ¡abre un issue o PR!

## 📚 Arquitectura Lean y Extensibilidad

El proyecto ha sido refactorizado bajo principios de **Alta Cohesión y Bajo Acoplamiento**:

- **Generalist Agent**: Un único motor de predicción lógica.
- **Super-Skills**: Proveedores configurables (OpenAI/Groq/Local) a través de `GenericHttpAiProvider`.
- **Unified Persistence**: `SettingsManager` centraliza perfiles y configuración con almacenamiento seguro.
- **Plugin System (ABI 1.0)**: Carga dinámica de proveedores como librerías compartidas (`.so` / `.dll`).

Documentación técnica:
- `agents.md` — Roles y comportamiento del agente generalista.
- `skills.md` — Contrato de habilidades y guía de desarrollo de plugins.
- `colabb-cpp/docs/examples/` — Plantillas de código y CMake para nuevos desarrollos.

## 📄 Licencia

MIT

## 🗒️ Bitácora de Refactorización (Febrero 2026)

- **Consolidación de IA**: Eliminación de `GroqProvider` y `OpenAIProvider` en favor de `GenericHttpAiProvider`. Reducción del 80% de lógica duplicada.
- **Unificación de Settings**: Fusión de `ConfigManager` y `ProfileManager` en un único `SettingsManager`.
- **Limpieza de Código**: Eliminación de archivos "extended", huérfanos y versiones duplicadas.
- **Estándar ABI 1.0**: Unificación de la interfaz de plugins multiplataforma.

Para un historial detallado, consulta `Bitacora.md`.
