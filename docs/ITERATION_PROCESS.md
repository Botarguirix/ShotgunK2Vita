# Proceso de entregas / Delivery process

## Español

La versión y nombre vigentes están en `SG_BUILD_VERSION` y `SG_BUILD_NAME` de CMake. El proceso procede de las iteraciones locales del proyecto Touhou 7.5.

1. Conservar todas las entregas existentes. No reutilizar ni sobrescribir `artifacts/iterations/<versión>/`. No incrementar versión por documentación sola.
2. Guardar evidencia recibida intacta fuera de Git, en `reports/iterations/<nueva-versión>/input/`, con procedencia, versión que falló, acción y resultado. Asociar cores a ELF sólo con identidad de compilación suficiente.
3. Definir causa, cambio y aceptación. Incrementar versión sólo para una nueva entrega ejecutable y comprobar que su carpeta no exista.
4. Ejecutar `bash scripts/check_iteration.sh` en Linux/WSL con datos privados en `private/game/`; guardar resultados en carpetas únicas y ampliar la regresión según el cambio. Las pruebas PC no prueban rendimiento o estabilidad en Vita.
5. Actualizar `STATUS_ITERATIONNN`, `INFORME_ROADMAP_ITERACIONNN`, ROADMAP, ITERATIONS, README e instalación. Separar evidencia, validación local y aceptación pendiente en hardware.
6. Compilar con `bash scripts/build.sh` y empaquetar con `python tools/package_test.py --vpk-only`. Conservar VPK, ELF/map, snapshot, evidencia, validación y hashes. `delivery-audit.json` identifica archivos y estado de Git; el commit base no identifica por sí solo un árbol con cambios.
7. Publicar código y documentación autorizados. Nunca publicar datos comerciales, APK, logs personales, cores ni partidas. Revisar el contenido antes de subir un ZIP. Conservar localmente las entregas retiradas del catálogo público.
8. Entregar enlace al VPK, causa/cambio, pruebas y recorrido concreto de consola. Conservar datos y saves al actualizar. No afirmar corrección en hardware ni mejora de FPS sin evidencia de consola.

Documentación y herramientas pueden actualizarse sin alterar el paquete original. Si el árbol actual difiere del snapshot de un VPK existente, indicarlo explícitamente; los símbolos pertenecen a su compilación original. Véase [BUILDING](BUILDING.md).

## English

Current version and name are defined by `SG_BUILD_VERSION` and `SG_BUILD_NAME` in CMake. This process derives from local Touhou 7.5 project iterations.

1. Preserve all existing deliveries. Never reuse or overwrite `artifacts/iterations/<version>/`. Do not increase versions for documentation alone.
2. Keep received evidence intact outside Git, under `reports/iterations/<new-version>/input/`, recording origin, failing version, triggering action and outcome. Match cores to ELF files only with sufficient build identity.
3. Define cause, change and acceptance. Increase the version only for a new executable delivery and check that its directory does not exist.
4. Run `bash scripts/check_iteration.sh` in Linux/WSL with private assets under `private/game/`; keep results in unique directories and extend regression as needed. PC tests do not prove Vita performance or stability.
5. Update `STATUS_ITERATIONNN`, `INFORME_ROADMAP_ITERACIONNN`, ROADMAP, ITERATIONS, README and installation instructions. Separate evidence, local validation and pending hardware acceptance.
6. Build with `bash scripts/build.sh` and package with `python tools/package_test.py --vpk-only`. Preserve VPK, ELF/map, snapshot, evidence, validation and hashes. `delivery-audit.json` identifies files and Git state; a base commit alone does not identify a modified working tree.
7. Publish authorized code and documentation. Never publish commercial assets, APKs, personal logs, cores or saves. Inspect ZIP contents before uploading. Retain locally any delivery removed from the public catalog.
8. Deliver the VPK link, cause/change, tests and a concrete console checklist. Preserve data and saves when updating. Do not claim hardware fixes or FPS gains without console evidence.

Documentation and tools may be updated without altering the original package. Explicitly state when the current tree differs from an existing VPK snapshot; symbols belong to their original build. See [BUILDING](BUILDING.md#english).
