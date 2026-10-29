# CompilaciÃ³n / Building â€” 0.78 alpha

## EspaÃ±ol

Linux/WSL: VitaSDK, CMake, make, Python 3 y dependencias incluidas. Steam requiere LuaJIT. Dependencias externas: hyln9/vita-luajit commit 0f0252ab8615bc4b097da0bfe0bf10d0666a3dcd y hyln9/vita-libdl commit bf46551882c344e5aebee07f15267e2ca93d78a5. Clona con finales de lÃ­nea LF, conserva sus licencias y compila segÃºn upstream. LuaJIT exige gcc-multilib/libc6-dev-i386 para sus generadores host de 32 bits. Con SDK actuales, elimina las declaraciones RWLock antiguas de libdl/source/dl.c: las aporta el header del SDK.

```sh
export VITASDK=/ruta/vitasdk
export PATH="$VITASDK/bin:$PATH"
python3 tools/generate_neutral_livearea.py
# Default paths: _local/vita-luajit-source-20261017 and _local/vita-libdl
SG_BUILD_DIR="$PWD/build-iterations/00.78-steam-alpha" bash scripts/build.sh -DSG_LUAJIT_EXPERIMENTAL=ON -DSG_STEAM_SGR_EXPERIMENTAL=ON -DVITA_LIVEAREA_DIR="$PWD/private/livearea"
SG_BUILD_DIR="$PWD/build-iterations/00.78-legacy-alpha" bash scripts/build.sh -DSG_LUAJIT_EXPERIMENTAL=ON -DSG_STEAM_SGR_EXPERIMENTAL=OFF -DVITA_LIVEAREA_DIR="$PWD/private/livearea"
python tools/package_test.py --vpk-only
```

SG_VITA_LUAJIT_ROOT y SG_VITA_LIBDL_ROOT permiten otras rutas. Si el workspace contiene espacios, crea un enlace sin espacios y exporta SG_SOURCE_DIR con ese enlace; configura un build nuevo. Los VPK pÃºblicos usan imÃ¡genes neutrales propias generadas por tools/generate_neutral_livearea.py, no arte del juego; define VITA_LIVEAREA_DIR con esa carpeta o deja el arte sin generar. Los scripts make_livearea/prepare_livearea procesan archivos externos del usuario y escriben en private/, nunca se adjunta su salida comercial a un release.

CMake genera build.../generated/code_patches.h. Sin datos privados contiene cero parches y el build funciona. Opcionalmente define SG_PATCH_ORIGINAL, SG_PATCH_OPTIMIZED y SG_PATCH_TARGET: tools/gen_code_patches.py valida los reemplazos a partir de tus tres archivos. No subas esos inputs, el header generado ni un VPK que los incorpore; los VPK pÃºblicos se construyen con las tres opciones vacÃ­as. El historial privado que contenÃ­a el header no forma parte de esta rama.

Pruebas: bash scripts/check_iteration.sh requiere datos 1.5 privados en private/game/. Las pruebas de instalaciÃ³n Steam reciben una copia local de data.sgr; los resultados PC no equivalen a hardware. Ver STATUS_ITERATION78.

## English

Use Linux/WSL, VitaSDK, CMake, make and Python 3. Steam requires LuaJIT. External dependencies: hyln9/vita-luajit at 0f0252ab8615bc4b097da0bfe0bf10d0666a3dcd and hyln9/vita-libdl at bf46551882c344e5aebee07f15267e2ca93d78a5. Keep LF source files and upstream licenses. LuaJIT's host generators require gcc-multilib/libc6-dev-i386. Current SDK headers replace the two obsolete RWLock forward declarations in libdl/source/dl.c.

Use the commands above for separate Steam/1.5 builds. SG_VITA_LUAJIT_ROOT and SG_VITA_LIBDL_ROOT override dependency locations. For paths with spaces, create a space-free symlink, export SG_SOURCE_DIR pointing to it, and configure a fresh build. Public VPKs use original neutral artwork from tools/generate_neutral_livearea.py; set VITA_LIVEAREA_DIR accordingly. Commercial output from optional artwork preparation scripts stays private.

CMake generates code_patches.h only in the ignored build tree. No private inputs means zero patches and a valid build. Optional SG_PATCH_ORIGINAL/OPTIMIZED/TARGET inputs generate local-only patches; never publish the inputs, generated header or a VPK containing them. Public builds leave all three inputs empty. The private history containing the header is excluded from this branch.

bash scripts/check_iteration.sh requires private 1.5 data in private/game/. Steam installation probes use a private copy of data.sgr. PC tests are not console acceptance; see STATUS_ITERATION78.
