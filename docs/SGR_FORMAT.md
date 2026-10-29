# Extractor Steam SGR / Steam SGR extractor

## Español

`tools/extract_sgr.py` abre el `data.sgr` de la edición Steam investigada sin ejecutar el juego ni necesitar el motor SUGAR instalado. Implementa el formato reconstruido del cargador, con XOR de bytes alternos, tamaño big-endian y zlib. Los recursos internos tienen otra capa del mismo XOR.

Comprobado con Shotgun King PC **1.623b**, Steam build **24554104**:

```text
data.sgr SHA-256:
9429956abcbe7246dd577aa9dad7ab7728740278ac7da0a1672b3a376e807e8c
EXE usado para investigar el cargador SHA-256:
50f5618e4109f714674ee4f8b2f64b96042152f79f701c32ea9e5deaaf522ffe
```

El extractor sólo requiere la biblioteca estándar de Python; el EXE no se necesita al extraer. Usar una carpeta de salida nueva y privada:

```powershell
python tools/extract_sgr.py "F:/SteamLibrary/steamapps/common/Shotgun King The Final Checkmate/data.sgr" private/steam-extraido
python tests/test_sgr_extractor.py
```

Salida:

- `files/`: recursos con nombres portables.
- `virtual/`: recursos cuyos nombres originales contienen rutas no portables, como `../../libs/...`; se renombran de manera neutral.
- `lists/`: los tres índices comodín observados, con rutas separadas por NUL.
- `duplicates/`: entradas con destino repetido; no se sobrescriben las anteriores.
- `manifest.json`: nombres originales, rutas físicas, flags, tamaños y hashes SHA-256.

El archivo investigado contiene 279 registros: 276 de recursos, con un duplicado idéntico, y tres índices comodín. Se recuperaron 27 scripts Lua; todos compilan sintácticamente en LuaJIT 2.1 y Lua 5.4 sin ejecutarlos. Lua 5.1 estándar rechaza `code/grid.lua` por `goto`/etiquetas. Los scripts no equivalen al código fuente C/C++ del motor ni a una preparación automática para Vita.

Validación: nueve pruebas sintéticas aprobadas, comparación exacta de la transformación contra la rutina x86 aislada (cinco semillas), dos rutas de extracción con los mismos 279 hashes, comprobación PNG/WAV y firmas OGG/fuentes. No se ejecutó el juego. La compatibilidad con otras ediciones SGR no se ha establecido.

La extracción Python puede tardar decenas de segundos. `--max-mib` limita entrada/salida descomprimida (512 MiB por defecto). `--decoded-container` sólo acepta un contenedor que ya haya pasado por XOR y zlib; no usar esa opción con el SGR original. El extractor rechaza sobrescribir una carpeta existente.

No publicar los datos extraídos. Mantener la salida fuera de Git y separada de datos activos y saves. No se requiere una nueva versión de VPK para esta herramienta de análisis.

## English

`tools/extract_sgr.py` opens the investigated Steam `data.sgr` without running the game or installing SUGAR. It implements the recovered loader format: XOR on alternating bytes, a big-endian size and zlib. Each embedded resource has another layer of the same XOR.

Verified with Shotgun King PC **1.623b**, Steam build **24554104**; input and executable identities are listed above. The extractor needs only Python's standard library; extraction does not require the EXE. Use a fresh private output directory:

```powershell
python tools/extract_sgr.py "F:/SteamLibrary/steamapps/common/Shotgun King The Final Checkmate/data.sgr" private/steam-extracted
python tests/test_sgr_extractor.py
```

Output:

- `files/`: resources with portable names.
- `virtual/`: neutral filenames for original nonportable paths such as `../../libs/...`.
- `lists/`: the three observed wildcard indexes, containing NUL-separated paths.
- `duplicates/`: repeated output entries, preserved without overwriting earlier records.
- `manifest.json`: original names, physical paths, flags, sizes and SHA-256 hashes.

This file contains 279 records: 276 resource records, including one identical duplicate, and three wildcard indexes. All 27 recovered Lua scripts compile syntactically in LuaJIT 2.1 and Lua 5.4 without execution. Stock Lua 5.1 rejects `code/grid.lua` because it uses goto/labels. These scripts are not C/C++ engine source or automatically prepared Vita game data.

Validation: nine passing synthetic tests, exact comparison with the isolated x86 transform using five seeds, identical hashes for all 279 records through two extraction paths, PNG/WAV checks and OGG/font signatures. The game was not run. Compatibility with other SGR editions is not established.

Python extraction may take tens of seconds. `--max-mib` limits input/decoded size (512 MiB by default). `--decoded-container` accepts only an already XOR-decoded and zlib-decompressed container; do not use it with the original SGR. Existing output directories are rejected.

Do not publish extracted commercial data. Keep output outside Git and separate from active game data and saves. This analysis tool does not require a new VPK version.
