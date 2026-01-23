# Colabb Terminal - C++ Edition

Una terminal moderna para Linux potenciada con Inteligencia Artificial, reescrita en C++ para mejor rendimiento y escalabilidad.

## 🚀 Características

- **Terminal Real**: Basada en VTE (GTK3), soporta todos tus comandos habituales
- **Multi-Tab**: Gestiona múltiples terminales en pestañas con atajos de teclado
- **Asistencia por IA**: Integración con Groq (Llama 3.1) y OpenAI
- **Sistema Totem (`?`)**: Escribe `?` seguido de tu consulta para invocar a la IA
- **Conciencia de Contexto**: La IA lee errores y salidas previas para sugerencias inteligentes
- **Autocompletado Rápido**: Aplica sugerencias con `Ctrl + Space`
- **Búsqueda en Terminal**: Busca texto con soporte para regex y sensibilidad a mayúsculas
- **Caché Inteligente**: Las sugerencias se cachean para respuestas instantáneas
- **Configuración Segura**: API Keys almacenadas con libsecret

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
    libsecret-1-dev
```

### Dependencias Opcionales

- Google Test (para tests): `sudo apt install libgtest-dev`

## 📦 Compilación

```bash
cd colabb-cpp
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## ▶️ Instalación

```bash
sudo make install
```

O ejecuta directamente desde el directorio build:

```bash
./colabb
```

## 🎯 Uso

1. **Configuración Inicial**:
   - Haz clic en el icono de engranaje (⚙️) en la esquina superior derecha
   - Selecciona tu proveedor de IA (groq/openai)
   - Pega tu API Key
   - Haz clic en "Guardar y Validar"

2. **Pedir ayuda a la IA**:
   - Escribe `?` seguido de tu consulta
   - Ejemplo: `? listar archivos ocultos`
   - Espera la sugerencia (aparece en la barra inferior)
   - Presiona `Ctrl + Space` para aplicar el comando
   - Presiona `Enter` para ejecutarlo

3. **Atajos de Teclado**:

   **Gestión de Pestañas:**
   - `Ctrl + Shift + T`: Nueva pestaña
   - `Ctrl + Shift + W`: Cerrar pestaña actual
   - `Ctrl + PageUp`: Pestaña anterior
   - `Ctrl + PageDown`: Pestaña siguiente

   **Búsqueda:**
   - `Ctrl + Shift + F`: Abrir/cerrar búsqueda
   - `F3`: Buscar siguiente
   - `Shift + F3`: Buscar anterior
   - `Escape`: Cerrar búsqueda

   **IA:**
   - `Ctrl + Space`: Aplicar sugerencia
   - `Escape`: Resetear estado de la IA

   **Terminal:**
   - `Ctrl + C/U/L`: Limpiar línea actual

## 🏗️ Arquitectura

El proyecto sigue una arquitectura en capas:

```
Infrastructure Layer (VTE, HTTP, Config)
         ↓
   Domain Layer (AI Providers, Models)
         ↓
 Application Layer (Prediction Service)
         ↓
Presentation Layer (GTK UI)
```

## 🧪 Testing

```bash
cd build
cmake -DBUILD_TESTING=ON ..
make
ctest
```

## 📁 Estructura del Proyecto

```
colabb-cpp/
├── src/
│   ├── infrastructure/  # VTE, HTTP, Config
│   ├── domain/          # AI providers, Models
│   ├── application/     # Business logic
│   ├── ui/              # GTK interface
│   └── main.cpp
├── include/
│   └── colabb/
│       └── version.hpp
├── tests/
└── CMakeLists.txt
```

## 🤝 Contribución

Si encuentras bugs o tienes ideas para nuevas features, ¡abre un issue o PR!

## 📄 Licencia

MIT

## 🔄 Migración desde Python

Si estás migrando desde la versión Python:

1. **Configuración**: Las API keys se migrarán automáticamente a libsecret
2. **Logs**: Ahora se guardan en `~/.colabb_session.log`
3. **Config**: El archivo de configuración está en `~/.config/colabb/config.json`

## 🐛 Troubleshooting

### Error: "Failed to spawn shell"

- Verifica que `/usr/bin/script` esté instalado
- Comprueba permisos en tu directorio home

### Error: "Failed to store API key"

- Asegúrate de que `libsecret` esté instalado
- Verifica que el servicio de keyring esté activo

### La IA no responde

- Verifica tu conexión a internet
- Comprueba que tu API key sea válida
- Revisa los logs en la terminal donde ejecutaste `colabb`
