# Dependencias del sistema – Seguridad y actualización

Para asegurar máxima estabilidad y seguridad en Colabb Terminal, revisa y actualiza periódicamente las dependencias del sistema listadas en este proyecto.

**Dependencias críticas:**

- GTK3
- VTE (vte-2.91)
- libcurl
- libsecret-1
- nlohmann/json (incrustado vía fetch, revisa releases upstream)
- GoogleTest (opcional, solo para tests)

**Recomendaciones:**

- Usa siempre versiones soportadas y con parches de seguridad activos en tu distribución.
- Reinstala dependencias con el gestor de paquetes del sistema, por ejemplo:

```sh
sudo apt update && sudo apt upgrade
sudo apt install --only-upgrade libgtk-3-dev libvte-2.91-dev libcurl4-openssl-dev libsecret-1-dev
```

- Para proyectos CMake, revisa frecuentemente si hay nuevas versiones de las releases incluidas vía `FetchContent` (por ejemplo, `nlohmann/json`).
- Considera también monitorear CVEs públicas de las librerías.

---

Esta advertencia resume mejores prácticas y reduce riesgos por dependencias obsoletas o vulnerables.
