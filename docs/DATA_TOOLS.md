# Datos y herramientas / Data and tools

## Español

Herramientas en `tools/` y el ZIP DataTools de la release. Requieren Python **3.11+** (el inspector APK usa `hashlib.file_digest`) y Pillow. Desde la raíz del repositorio:

```sh
python -m pip install -r tools/requirements.txt
python tools/inspect_apk.py /ruta/juego.apk --output reports/apk-inspection.json
python tools/prepare_data.py /ruta/juego.apk private/game
```

El APK debe contener scripts Lua y recursos legibles en `assets/`, incluido `assets/assets/gfx/gfx.png`. El resultado debe contener `private/game/code.lua` y `private/game/assets/gfx/gfx.png`. El preparador copia los tipos admitidos y genera `.sgimg`; no ejecuta ni copia bibliotecas nativas. No hay una versión comercial exacta certificada como universalmente compatible.

Prepara datos en una carpeta nueva. Copia su contenido a `ux0:data/shotgunk2vita/`, excluyendo `save/` si ya tienes partidas. No publiques el resultado, APK, música, scripts comerciales ni saves.

Estas herramientas **no son un decompilador general**. `prepare_data.py` extrae archivos ya presentes en el APK. `inspect_pe.py` inspecciona cabeceras, símbolos e imports; no extrae `data.sgr`. Tener sólo el EXE o una compra de Steam no basta para ejecutar el preparador APK.

| Herramienta | Función |
|---|---|
| `prepare_data.py` | Extraer y preparar recursos APK |
| `inspect_apk.py` | Inspeccionar arquitectura y recursos sin ejecutar el juego |
| `inspect_pe.py` | Inspección estática de EXE/DLL locales |
| `map_sugar_api.py` | Comparar llamadas Lua con un manual SUGAR local mediante heurísticas |
| `prepare_livearea.py` | Generar PNG indexados desde dos imágenes propias |

```sh
python tools/inspect_pe.py /ruta/shotgun_king.exe --output reports/pe-inspection.json
python tools/map_sugar_api.py /ruta/juego.apk /ruta/manual-sugar.txt --output reports/sugar-map.json
python tools/prepare_livearea.py /ruta/icono.png /ruta/fondo.png --out private/livearea
```

LiveArea es opcional al compilar; no prepara datos jugables. Su interfaz actual acepta imágenes, no un EXE. Los informes permanecen locales en `reports/`.

## English

Tools are in `tools/` and the release DataTools ZIP. Requires Python **3.11+** (`inspect_apk.py` uses `hashlib.file_digest`) and Pillow. From the repository root:

```sh
python -m pip install -r tools/requirements.txt
python tools/inspect_apk.py /path/game.apk --output reports/apk-inspection.json
python tools/prepare_data.py /path/game.apk private/game
```

The APK must contain readable Lua scripts and assets under `assets/`, including `assets/assets/gfx/gfx.png`. Output must include `private/game/code.lua` and `private/game/assets/gfx/gfx.png`. The preparer copies supported types and generates `.sgimg`; it does not execute or copy native libraries. No exact commercial version is certified as universally compatible.

Prepare data in a new directory. Copy its contents to `ux0:data/shotgunk2vita/`, excluding `save/` if saves exist. Do not publish output, APK, music, commercial scripts or saves.

These tools are **not a general decompiler**. `prepare_data.py` extracts existing APK files. `inspect_pe.py` inspects headers, symbols and imports; it does not extract `data.sgr`. An EXE or Steam purchase alone is not input for the APK preparer.

| Tool | Purpose |
|---|---|
| `prepare_data.py` | Extract and prepare APK assets |
| `inspect_apk.py` | Inspect architecture and assets without executing the game |
| `inspect_pe.py` | Statically inspect local EXE/DLL files |
| `map_sugar_api.py` | Heuristically compare Lua calls with a local SUGAR manual |
| `prepare_livearea.py` | Generate indexed PNGs from two user images |

```sh
python tools/inspect_pe.py /path/shotgun_king.exe --output reports/pe-inspection.json
python tools/map_sugar_api.py /path/game.apk /path/sugar-manual.txt --output reports/sugar-map.json
python tools/prepare_livearea.py /path/icon.png /path/background.png --out private/livearea
```

LiveArea is optional when building; it does not prepare playable data. Its current interface accepts images, not an EXE. Reports stay local under `reports/`.
