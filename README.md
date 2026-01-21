# Colabb Terminal IA

Una terminal moderna para Linux potenciada con Inteligencia Artificial. Combina un emulador de terminal real (VTE) con la capacidad de predecir y generar comandos complejos usando lenguaje natural.

## 🚀 Características

- **Terminal Real**: Basada en `Vte.Terminal` (GTK3), soporta todos tus comandos habituales, colores y programas interactivos (vim, htop, etc.).
- **Asistencia por IA**: Integración con **Groq** (Llama 3.1) para respuestas instantáneas.
- **Sistema Totem (`?`)**: Escribe `?` seguido de tu consulta para invocar a la IA sin interferir con tu flujo de trabajo normal.
  - Ejemplo: `? descomprimir tar.gz` -> Sugerencia: `tar -xzvf archivo.tar.gz`
- **Autocompletado Rápido**: Aplica la sugerencia inmediatamente presionando `Ctrl + Space`.
- **Configuración Segura**: Gestión de API Keys encriptadas localmente.

## 🛠️ Requisitos

- Python 3.x
- GTK 3.0 y VTE 2.91
- Librerías Python: `PyGObject`, `requests`, `cryptography`

## 📦 Instalación

1.  Clonar el repositorio:

    ```bash
    git clone https://github.com/Medalcode/Colabb.git
    cd Colabb
    ```

2.  Instalar dependencias del sistema (Debian/Ubuntu):

    ```bash
    sudo apt install python3-gi python3-gi-cairo gir1.2-gtk-3.0 gir1.2-vte-2.91
    ```

3.  Instalar dependencias de Python:
    ```bash
    pip3 install -r requirements.txt
    ```

## ▶️ Uso

1.  Ejecuta la aplicación:

    ```bash
    python3 main.py
    ```

2.  **Configuración**:
    - Haz clic en el icono de engranaje (⚙️) en la esquina superior derecha.
    - Selecciona "groq" y pega tu API Key.
    - Guarda y valida la conexión.

3.  **Pedir ayuda a la IA**:
    - En la terminal, escribe `?` seguido de tu instrucción.
    - Observa la barra inferior "Analizando...".
    - Cuando aparezca la sugerencia, presiona **`Ctrl + Space`** para reemplazar tu texto con el comando sugerido.

## 🤝 Contribución

Si encuentras bugs o tienes ideas para nuevas features, ¡abre un issue o PR!

## 📄 Licencia

MIT
