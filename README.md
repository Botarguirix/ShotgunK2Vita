# ShotgunK2Vita

Port experimental de Shotgun King a PS Vita. El runtime propio interpreta los
scripts Lua del APK con Lua 5.1 y una implementación parcial de la API SUGAR.
El usuario confirmó el arranque de 00.02 en Vita OLED a 1–3 FPS y sin sonido.

La versión actual **00.11 — GPU Presenter** compone 320×180 y escala por
GPU, conservando L1/R1 táctiles y métricas internas sin HUD. Corrige fb_ev()
ausente al desbloquear logros; desbloqueo y guardado pasan en PC. La sesión
00.10 recibió 17,80 FPS de media activa. Compilación Vita y regresiones locales
válidas; mejora de FPS y presentación GXM pendientes de probar en la consola.

- [Roadmap y evidencia](docs/ROADMAP.md)
- [Proceso estándar de iteraciones](docs/ITERATION_PROCESS.md)
- [Informe de la iteración actual](docs/INFORME_ROADMAP_ITERACION11.md)
- [Instalación del VPK 00.11](docs/TEST_BUILD.md)
- [Recuperar logs tras un crasheo](docs/CRASH_LOGS.md)
- [Herramienta de rendimiento](docs/PERFORMANCE_TOOL.md)
- [Controles para Vita](docs/images/controls-vita.png)

No es un port completo. Continúan pendientes fidelidad gráfica, todas las
funciones SUGAR, otros modos/cartas, audio avanzado, saves anteriores y CRT.
Los archivos comerciales del juego se preparan localmente y no se publican.

## Compilar

Requiere VitaSDK, CMake y Linux/WSL. Lua, stb_truetype y dr_mp3 están incluidos.

```sh
export VITASDK=/home/bota/vitasdk
bash scripts/build.sh
```

La corrección **00.04** convierte LiveArea a PNG indexados opacos de 8 bits,
con paleta de 256 entradas, para resolver el error de instalación 0x8010113D.
El usuario confirma mejora considerable de FPS tras 00.04 y reporta un crasheo.
**00.05** añade logs por sesión que no se sobrescriben, checkpoints ~250 ms,
trazas de errores Lua y símbolos nativos. Falta diagnosticar la causa del crash.
El log recibido identifica una llamada a `delta()` no registrada en la
repetición del pad. **00.06** añade ese alias de `dt()`; reproduce el error
anterior y verifica direcciones mantenidas de cruceta/stick en PC.

**00.07** implementa `trifill()` y `tri()` para corregir la parada al usar
el stick derecho. El recorrido de 5.400 frames pasa en PC con sanitizadores;
la validación en Vita sigue pendiente. [Diagnóstico y pruebas](docs/STATUS_ITERATION07.md).

**00.08** corrige `aspr()` ausente al disparar y los sliders de opciones,
añade doble framebuffer con composición en RAM y optimiza dibujo de texto.
Pasan 6000 frames de disparo/recarga en PC; FPS y parpadeo deben verificarse
en Vita. [Diagnóstico](docs/STATUS_ITERATION08.md) y [auditoría](docs/STABILITY_AUDIT.md).

Salida: `build-iterations/00.10/ShotgunK2Vita-performance.vpk`, título
`SGKV00001`, versión 00.10. Cada versión usa una carpeta de compilación propia.
El workflow compila este VPK sin datos comerciales ni arte extraído localmente.

## Preparar datos e imágenes de tu copia

Requiere Python 3.9+ y Pillow. El preparador extrae recursos y scripts;
no ejecuta las bibliotecas nativas del APK ni el EXE.

```sh
python3 tools/prepare_data.py /ruta/juego.apk private/game
python3 tools/prepare_livearea.py /ruta/shotgun_king.exe private/game private/livearea
```

Compilar después de generar LiveArea. El icono procede de RT_ICON del EXE;
la portada usa el atlas de título del APK. Si `private/livearea/` no existe,
la compilación omite ese arte. Cada PNG de datos recibe un `.sgimg` auxiliar.
Los datos de 00.02 siguen siendo válidos; no funciona copiando sólo `data.sgr`.

```sh
python3 tools/package_test.py
```

Genera VPK, copia del código/documentación, validación y hashes en
`artifacts/iterations/00.10/`. Incluye ELF/map con símbolos en `debug/`.
Cada versión ocupa su propia carpeta y el
empaquetador rechaza sobrescribir una entrega existente. Nunca reemplazar
`save/` de la consola al actualizar únicamente el ejecutable.
Para actualizar sólo el VPK: `python3 tools/package_test.py --vpk-only`.

## Pruebas en PC

```sh
cmake -S . -B build-host -DSG_HOST_PROBE=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-host -j4
mkdir -p reports
./build-host/sg_probe private/game 4800 game
./build-host/sg_bench private/game reports/perf.csv 4800 reports/frame.ppm 3
./build-host/sg_audio_probe private/game reports/audio.wav
```

La prueba simula botones y registra el estado. El benchmark presenta cada
fotograma sin VSync; sus tiempos/FPS derivados no son resultados de Vita.
La prueba de audio simula 300 segundos, verifica pista A→B, bucle, efectos y
silencio; guarda ocho segundos de PCM para inspección. No confirma salida audible
por el puerto de la consola.

## Investigación

El APK recibido contiene exclusivamente bibliotecas ARM64. El runtime usa los
scripts y recursos, sin cargar esos binarios. Hallazgos:
[viabilidad](docs/FEASIBILITY.md), [Steam](docs/STEAM_ANALYSIS.md),
[recursos GitHub](docs/GITHUB_RESOURCES.md).
La API se implementa a partir de la
[documentación pública SUGAR](https://github.com/TRASEVOL-DOG/Shotgun-King-Modding-Guide).
Dependencias y licencias: [third_party](third_party/README.md).
