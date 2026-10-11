# Entregas locales por iteración

| Versión | Carpeta | Estado |
|---|---|---|
| 00.02 | `artifacts/iterations/00.02/` | Bootstrap; arranque confirmado, 1–3 FPS y sin audio |
| 00.03 | `artifacts/iterations/00.03/` | Rendimiento/audio; usuario reporta 0x8010113D |
| 00.04 | `artifacts/iterations/00.04/` | Usuario reporta mejora considerable de FPS, pero crasheo; sin medición exacta aún |
| 00.05 | `artifacts/iterations/00.05/` | Logs persistentes por ejecución, traceback Lua y símbolos nativos; pendiente de prueba en Vita |

Los archivos originales de 00.02 y 00.03 permanecen además en `artifacts/`.
La entrega 00.03 conserva una copia del código publicado en ese momento.
00.04 y siguientes incluyen VPK, snapshot de código y documentación, herramienta,
configuración, validación y hashes en la carpeta de su versión. Los datos
comerciales compartidos siguen en el ZIP original; no requieren otra copia
en la consola para esta corrección.

`scripts/build.sh` selecciona `build-iterations/<VERSION>/` según CMake.
`tools/package_test.py` valida el VPK antes de entregarlo y se niega a reemplazar
una carpeta de entrega existente. Para una nueva entrega, incrementar VERSION
en CMake y actualizar el estado de esta tabla. Las carpetas de compilación son
reutilizables dentro de su versión; las entregas empaquetadas se conservan.

La validación local comprueba ZIP/CRC, SFO/título/versión, dimensiones PNG,
tipo de color indexado, profundidad de 8 bits, paleta completa sin transparencia
y referencias de LiveArea. Esto no confirma instalación ni rendimiento en Vita.
