# Bitácora de Cambios

Fecha: 2026-01-27

Proyecto: Colabb Terminal (C++ / GTK3)

Resumen de la sesión:

- Implementé un LRU para `SuggestionCache` (mejora de rendimiento y escalabilidad).
  - Archivos modificados: `src/application/suggestion_cache.hpp`, `src/application/suggestion_cache.cpp`.
  - Motivo: `evict_oldest()` era O(n) y degradaba con el tamaño de la caché.
  - Estado: COMPLETADO

- Añadí un límite configurable de cola a `PredictionService` (`max_queue_size`) y política
  de rechazo inmediato cuando la cola está llena.
  - Archivos modificados: `src/application/prediction_service.hpp`, `src/application/prediction_service.cpp`.
  - Motivo: prevenir crecimiento ilimitado de memoria por ráfagas de peticiones.
  - Estado: COMPLETADO

- Añadí tests unitarios y ejecuté la suite de tests.
  - Tests añadidos: `tests/unit/prediction_service_queue_test.cpp`.
  - Tests existentes actualizados/ejecutados: `tests/unit/suggestion_cache_test.cpp`.
  - Resultado: TODOS LOS TESTS PASARON (18/18), ver `ctest` en `colabb-cpp/build`.
  - Estado: COMPLETADO

Cambios pendientes / recomendaciones (próximos pasos sugeridos):

1. Extraer `create_ai_provider()` a `infrastructure/ai_factory` para desacoplar UI de la
   creación de proveedores y facilitar pruebas. (Riesgo bajo, esfuerzo ~1–3h).
2. Refactorizar la lógica de sugerencias en `SuggestionController` para reducir la responsabilidad
   de `MainWindow`. (Mayor esfuerzo, 4–8h; hacer incrementalmente).
3. Añadir tests de concurrencia para `PredictionService` y, si procede, hacer `SuggestionCache`
   thread-safe si se accede desde workers.

Notas de integración:

- La suite de tests se ejecuta con CMake/CTest. Comandos:

```bash
cd colabb-cpp
cmake -S . -B build
cmake --build build -- -j$(nproc)
ctest --test-dir build --verbose
```

- Evitar cambios masivos en la UI o introducir DI completo hasta tener más tests de integración.

Registro de commits relevantes (local):

- Implementación LRU `SuggestionCache` — archivo modificado.
- Límite de cola en `PredictionService` — archivo modificado.
- Test `prediction_service_queue_test.cpp` añadido.

---

Esta bitácora será actualizada conforme avance la evolución incremental.
