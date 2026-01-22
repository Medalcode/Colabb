# Colabb Terminal

Una terminal moderna para Linux potenciada con Inteligencia Artificial, escrita en C++ para máximo rendimiento y escalabilidad.

## 🚀 Características

- **Terminal Real**: Basada en VTE (GTK3), soporta todos tus comandos habituales
- **Asistencia por IA**: Integración con Groq (Llama 3.1) y OpenAI
- **Sistema Totem (`?`)**: Escribe `?` seguido de tu consulta para invocar a la IA
- **Conciencia de Contexto**: La IA lee errores y salidas previas para sugerencias inteligentes
- **Autocompletado Rápido**: Aplica sugerencias con `Ctrl + Space`
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

## 📦 Compilación e Instalación

```bash
cd colabb-cpp
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

O ejecuta directamente:

```bash
./colabb
```

## ▶️ Uso

1. **Configuración Inicial**:
   - Haz clic en el icono de engranaje (⚙️)
   - Selecciona tu proveedor de IA (groq/openai)
   - Pega tu API Key
   - Haz clic en "Guardar y Validar"

2. **Pedir ayuda a la IA**:
   - Escribe `?` seguido de tu consulta
   - Ejemplo: `? listar archivos ocultos`
   - Presiona `Ctrl + Space` para aplicar
   - Presiona `Enter` para ejecutar

3. **Atajos de Teclado**:
   - `Ctrl + Space`: Aplicar sugerencia
   - `Escape`: Resetear estado
   - `Ctrl + C/U/L`: Limpiar línea

## 📁 Estructura del Proyecto

```
colabb-cpp/          # Implementación C++ (actual)
legacy-python/       # Implementación Python (deprecated)
```

## 🤝 Contribución

Si encuentras bugs o tienes ideas, ¡abre un issue o PR!

## 📄 Licencia

MIT

---

**Nota**: La versión Python ha sido movida a `legacy-python/` y ya no se mantiene activamente. Se recomienda usar la versión C++ para mejor rendimiento y estabilidad.
