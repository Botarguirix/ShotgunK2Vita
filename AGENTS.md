# Proceso de iteraciones ShotgunKing2Vita

Seguir docs/ITERATION_PROCESS.md para cada entrega, adaptado del proyecto local
Touhou 7.5. La versión y nombre vigentes están en SG_BUILD_VERSION y SG_BUILD_NAME de CMake.

- Conservar todas las entregas existentes: nunca reutilizar ni sobrescribir
  artifacts/iterations/<versión>. No incrementar versión por documentación sola.
- Guardar logs, cores y capturas recibidos fuera de Git, en reports/iterations/
  <nueva-versión>/input/. Conservar procedencia y versión que produjo el fallo.
- Diagnosticar con evidencia; comparar el caso fallido antes/después cuando sea
  posible. Asociar un core a un ELF sólo con identidad de compilación suficiente.
- Ejecutar comprobaciones pertinentes y guardar sus resultados. Usar
  scripts/check_iteration.sh para la regresión común; ampliar según el cambio.
- Actualizar STATUS_ITERATIONNN, INFORME_ROADMAP_ITERACIONNN, ROADMAP e ITERATIONS,
  además de las instrucciones de instalación y versión actual del README.
- Compilar con scripts/build.sh y entregar con tools/package_test.py --vpk-only.
  Incluir VPK, ELF/map, snapshot, evidencia, validación y hashes. No publicar
  datos comerciales, logs personales, cores ni partidas en GitHub.
- Distinguir pruebas PC, comprobación estática Vita y resultados de hardware.
  No declarar corregido en hardware o más FPS sin una prueba de consola.
- Entrega final: enlace absoluto al VPK, causa/cambio, pruebas y recorrido
  concreto que debe verificar el usuario; conservar datos y saves al actualizar.
