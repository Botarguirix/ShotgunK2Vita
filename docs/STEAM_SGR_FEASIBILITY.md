# Rama experimental Steam SGR / Experimental Steam SGR branch

## Español

La rama `codex/steam-sgr-feasibility` conserva el runtime propio en **C** y utiliza los scripts del juego mediante **LuaJIT**. Cambia la fuente de datos: `data.sgr` de Steam PC **1.623b**, sin APK. El puente Steam está desactivado por defecto (`SG_STEAM_SGR_EXPERIMENTAL=OFF`). La base se tomó del runtime local actual; el checkout original no se modificó.

**Resultado:** es viable preparar los datos en PC y ejecutar pruebas del runtime con ellos. No se ha entregado un VPK Steam ni se ha demostrado su funcionamiento en consola.

### Qué está implementado

- `tools/extract_sgr.py`: extracción del SGR investigado.
- `tools/prepare_steam_data.py`: extracción, normalización segura de rutas virtuales, imágenes `.sgimg`, conversión OGG→MP3 y perfil de recursos. Entrada comercial necesaria: únicamente `data.sgr`.
- `src/steam_compat.lua`: adapta contratos de superficies, fuentes, carga de audio, patrones de recursos y una cola explícita de texto. No modifica los 27 scripts originales en disco.
- Extensiones nativas opcionales para fuentes de respaldo y `strheight()`.
- `scripts/check_steam_data.sh`: pruebas PC reproducibles con saves nuevos, separados para menú y partida.

El dataset incluye el código y contenido completos de Steam. El puente sólo adapta APIs: **no selecciona automáticamente qué cambios de balance, cartas o modos incorporar**. No se fusionó esta rama a la base ni se sustituyeron datos activos del port.

### Pasadas realizadas

| Pasada | Resultado observado |
|---|---|
| Preparación desde el SGR | 275 recursos únicos, 20 imágenes, 26 pistas convertidas; 27 Lua intactos |
| Runtime sin puente | Se detiene en `psyms` |
| Puente inicial | Carga los módulos; se detiene en fallback de fuentes |
| Fuentes adaptadas | Llega al almacenamiento cifrado no implementado |
| Almacenamiento diagnóstico separado | `_init` completo |
| Primeros fotogramas | Bloqueo en dibujo del enlace externo |
| Política existente de retirar enlaces externos | 416 fotogramas; bloqueo en `strheight()` sin argumentos |
| Contrato de altura corregido | 600 fotogramas de menú aprobados |
| Primer ensayo de partida | Bloqueo en `catchtxt` |
| Cola de texto explícita | 1.200 fotogramas Throne aprobados; `ingame=true`, `playing=true` |
| Repetición con saves nuevos por prueba | Menú 600 + partida 1.200 + mezcla de audio offline aprobados |

El audio offline simula 300 segundos: dos inicios de música, tres bucles, cinco efectos y cero fallos de decodificación o desbordamientos de cola. No confirma sonido audible en Vita.

Pruebas adicionales: nueve del extractor, tres de preparación, contratos del puente y consultas de fuentes aprobados. La regresión común ASan/UBSan se ejecutó en una copia sin saves del dataset anterior. Tras actualizar el test obsoleto de las zonas traseras, pasa iteradores, Game Over, copia e inputs, pero se detiene en `test_input_cache`: espera dead zone 32, mientras el runtime base ya usa 48 y un umbral de flanco 44. No se declara la regresión común aprobada; el resto quedó sin ejecutar.

### Repetir la preparación

Requiere Python, Pillow y un ejecutable FFmpeg. Desde la raíz de esta rama:

```powershell
python -m pip install -r tools/requirements.txt
python tools/prepare_steam_data.py "F:/SteamLibrary/steamapps/common/Shotgun King The Final Checkmate/data.sgr" private/steam-nuevo --ffmpeg "C:/ruta/ffmpeg.exe"
python tests/test_sgr_extractor.py
python tests/test_steam_preparation.py
```

La salida debe ser una carpeta nueva. La herramienta rechaza sobrescribirla. No apunta al directorio de datos ni a las partidas actuales de Vita o Steam. Para repetir una evaluación se debe usar el adaptador de esta revisión; volver a preparar el dataset después de cambiar `src/steam_compat.lua`.

Para las pruebas host se compiló LuaJIT nativo **2.0.5**, a partir del fork `hyln9/vita-luajit` (referencia `0f0252ab8615bc4b097da0bfe0bf10d0666a3dcd`), en `_local/host-luajit`. Debe existir `src/libluajit.a` y `src/luajit`, compilados para el PC Linux/WSL; no usar la biblioteca ARM de Vita. Seguir las instrucciones de compilación upstream con `BUILDMODE=static`. El CMake acepta su raíz mediante `SG_HOST_LUAJIT_ROOT`.

Desde Linux/WSL, con CMake, GCC y make:

```bash
bash scripts/check_steam_data.sh private/steam-nuevo /ruta/luajit-nativo
```

Los resultados quedan en una carpeta única dentro de `reports/steam-feasibility/`. El script activa el puente explícitamente y usa LuaJIT host. La prueba de partida incluye pulsaciones simuladas; no demuestra por sí sola aceptación de cada acción ni todos los modos/cartas.

### ¿Puede leer directamente data.sgr en Vita?

Todavía no. El runtime actual consume archivos preparados. Esta rama demuestra la ruta **SGR → preparación en PC → datos del runtime**.

Una carga íntegra ingenua requiere mantener unos **169,9 MiB** entre el archivo comprimido y el contenedor expandido, antes de Lua, imágenes y audio. El `main.c` base declara un heap de **96 MiB**. Por tanto esa estrategia no cabe en ese heap. Habría que diseñar streaming, un caché extraído en almacenamiento o un sistema de archivos virtual, además de adaptar las lecturas de recursos. No se ha implementado ni medido una de esas alternativas en consola.

### Límites que siguen abiertos

- Sólo pruebas PC; sin VPK Steam y sin validación hardware.
- Guardados **de diagnóstico en texto plano**, no importación/compatibilidad con saves cifrados Steam. Los tests usan carpetas nuevas y no importan partidas existentes.
- El SGR sólo trae `lang/safe_english.txt`; no se han añadido traducciones desde el APK ni se promete español con el SGR solo.
- `watch` hace carga inicial única, no vigilancia de cambios en vivo. La cola de texto no está conectada a un teclado de escritorio.
- No están certificados mods, Charnier, todas las cartas, fidelidad gráfica ni todos los controles.
- No se promete más FPS. Los cambios Lua Steam pueden dejar sin usar aceleradores específicos de los scripts anteriores.
- Antes de un VPK de prueba: resolver persistencia, auditar APIs pendientes y regresiones, separar Title ID y ruta de datos, asignar una iteración nueva y empaquetar sin sobrescribir entregas.

No hay incremento de versión ni modificación de artefactos anteriores por esta evaluación de herramientas y host. Los recursos, logs, audio de prueba y saves siguen fuera de Git. Referencia del formato: [SGR_FORMAT](SGR_FORMAT.md).

## English

Branch `codex/steam-sgr-feasibility` retains the custom **C runtime** and runs the game's scripts with **LuaJIT**. It uses Steam PC **1.623b `data.sgr`** as the only commercial input, without an APK. The compatibility bridge is opt-in and disabled by default. The original checkout remains untouched.

The PC preparation path is demonstrated: 275 unique resources, 20 prepared images, 26 converted tracks and 27 unchanged Lua scripts. `tools/prepare_steam_data.py` normalizes virtual paths, generates SGI images, converts OGG to MP3 using FFmpeg and creates a resource profile. `src/steam_compat.lua` adapts API contracts; native opt-in additions provide fallback fonts and text height.

Repeated PC checks passed 600 menu frames, 1,200 Throne frames with active gameplay state and offline audio mixing. The audio test simulated 300 seconds with two music starts, three loops, five effects, no decoding failures and no queue drops. Nine extractor tests, three preparation tests, bridge contracts and native font queries also passed. These are not Vita hardware or audible-output results.

The common ASan/UBSan regression against a separate copy of previous assets still fails at a pre-existing input-cache test expecting a dead zone of 32, while the baseline runtime uses 48 with edge threshold 44. Earlier mutation-safe iteration, Game Over, frame copy and input checks passed after updating stale rear-pad test expectations. Remaining common checks did not run.

To prepare data, use the Python command above with a fresh private output folder and an explicit FFmpeg path. For host checks, supply a native Linux/WSL LuaJIT build with `src/libluajit.a` and `src/luajit` to `scripts/check_steam_data.sh`. The tested fork builds LuaJIT 2.0.5; do not substitute a Vita ARM archive. Each test receives a new save directory.

The runtime does not directly load raw SGR on Vita. A naive compressed-plus-expanded allocation is about 169.9 MiB, above the current 96 MiB heap before other resources. Streaming, a disk cache or virtual filesystem remains future work.

Limits: diagnostic plaintext saves only, no Steam encoded-save import; English fallback only from the SGR; single-shot asset watch; text queue without a desktop keyboard hook; incomplete full-game, mod, graphics and control acceptance. The dataset contains full Steam game content, not an automatic selection of isolated gameplay improvements. The original game scripts remain unchanged on disk, while existing platform guards may patch loaded chunks in memory. No FPS gain is claimed.

No Steam VPK was built or published and earlier deliveries remain intact. A future Vita candidate needs separate identity/data paths, persistence work, API/regression review and a new immutable iteration. Keep assets, private logs and saves outside Git.
