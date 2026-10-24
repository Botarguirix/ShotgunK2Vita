# Proceso estándar de iteraciones

Adaptado de los scripts build/check, carpeta input, auditorías y los informes
STATUS_ITERATION e INFORME_ROADMAP_ITERACION del proyecto local Touhou 7.5.
Se conserva la numeración SFO 00.NN y las carpetas ya usadas en ShotgunK2Vita.

1. Recibir evidencia y determinar qué versión produjo el fallo. Guardar una
   copia intacta en `reports/iterations/<nueva-versión>/input/`, con procedencia,
   acción que lo dispara y resultados observados. Un core necesita el ELF de
   su propia compilación; el sufijo `.tmp` no determina por sí solo su validez.
2. Definir causa, cambio y prueba de aceptación. Para una entrega nueva,
   incrementar `SG_BUILD_VERSION` y asignar `SG_BUILD_NAME` en CMake; comprobar que no exista ya su
   carpeta de entrega. Los cambios de documentación no requieren otro VPK.
3. Implementar y comprobar. En Linux/WSL ejecutar
   `bash scripts/check_iteration.sh`: triángulos, stick derecho, disparo/recarga,
   opciones, presentación, repetición del pad y logs con ASan/UBSan. Cada ejecución
   conserva logs en una carpeta única. Añadir pruebas específicas cuando sean
   necesarias; los datos privados preparados deben existir en `private/game`.
4. Documentar en `docs/STATUS_ITERATIONNN.md`: evidencia recibida, causa,
   corrección, validación local, límites, paquete y prueba pendiente. Crear
   `docs/INFORME_ROADMAP_ITERACIONNN.md`: progreso confirmado, trabajo actual,
   pendientes y siguiente prueba. Actualizar ROADMAP, ITERATIONS, README e
   instrucciones de instalación. Registrar cifras con ventana y plataforma.
5. Compilar con VitaSDK mediante `bash scripts/build.sh`. Se conserva
   `build-iterations/00.NN/` con VPK, ELF y mapa. Actualizar controles o LiveArea
   cuando el cambio lo requiera y comprobar sus formatos.
6. Empaquetar con `python tools/package_test.py --vpk-only`. Rechaza entregas
   existentes y verifica ZIP/CRC, SFO y LiveArea. Conserva snapshot, símbolos,
   herramientas, informes y evidencia. `delivery-audit.json` registra hashes
   de todos los archivos anteriores al audit, commit y modificaciones presentes
   al empaquetar; `unavailable` indica que Git no pudo consultarse. El audit no
   se incluye a sí mismo. Los hashes antiguos se mantienen por compatibilidad.
7. Publicar únicamente código y documentación autorizados. Los datos del juego,
   pruebas con recursos privados, logs y dumps permanecen fuera de GitHub.
   Si había cambios al empaquetar, el snapshot y los hashes describen la fuente
   entregada; el commit base por sí solo no identifica todos esos cambios.
8. Entregar enlace al VPK, cambio y pruebas realizadas. Definir acciones que
   debe probar el usuario y archivos que debe recuperar. La validación hardware
   queda pendiente hasta recibir evidencia. Registrar resultados posteriores en
   documentos de trabajo y en la entrada de la siguiente iteración, conservando
   intacta la entrega anterior.

Estructura de cada entrega futura:

```text
artifacts/iterations/00.NN/
  ShotgunK2Vita-iterationNN-nombre.vpk
  iteration.json
  debug/ShotgunK2Vita.elf + ShotgunK2Vita.map
  source/                       código/documentación del paquete
  evidence/                     entrada y resultados locales
  STATUS_ITERATIONNN.md
  INFORME_ROADMAP_ITERACIONNN.md
  validation.json + SHA256-performance.json
  delivery-audit.json
  ShotgunK2Vita-DiagnosticsTool.zip
```

Las entregas 00.02–00.07 conservan su estructura histórica. Este estándar se
aplica al siguiente empaquetado; no altera el VPK 00.07 ni sus hashes.

Pruebas PC y emulador: seguir [BUILDING.md](BUILDING.md).
Vita3K complementa la regresión y no sustituye la aceptación en consola.
Conservar perfiles de prueba separados y no incrementar versión por cambios
exclusivos en documentación o herramientas de pruebas.
