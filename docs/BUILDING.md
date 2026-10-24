# Compilación / Building

## Español

Requiere Linux/WSL, VitaSDK, CMake, make y compilador C. Las dependencias incluidas conservan sus licencias en [third_party](../third_party/README.md). Compilación predeterminada con Lua 5.1:

```sh
export VITASDK=/ruta/vitasdk
bash scripts/build.sh
```

Salida: `build-iterations/00.78/ShotgunK2Vita-performance.vpk`. Sin `private/livearea/` se omite el arte privado. Esta compilación predeterminada **no reproduce** el VPK LuaJIT existente.

La entrega 00.78 se compiló con `SG_LUAJIT_EXPERIMENTAL=ON`. Necesita [hyln9/vita-luajit](https://github.com/hyln9/vita-luajit), referencia `0f0252ab8615bc4b097da0bfe0bf10d0666a3dcd`, en `_local/vita-luajit-source-20261017/`, y [hyln9/vita-libdl](https://github.com/hyln9/vita-libdl), `bf46551882c344e5aebee07f15267e2ca93d78a5`, en `_local/vita-libdl/`, compilados según upstream. También enlaza `taihen_stub`. No se garantiza reproducción binaria: los cambios locales de estas dependencias no se han auditado en esta publicación.

```sh
cmake -S . -B build-iterations/00.78 -DSG_LUAJIT_EXPERIMENTAL=ON -DCMAKE_BUILD_TYPE=Release
bash scripts/build.sh
```

Regresión PC con datos privados: `bash scripts/check_iteration.sh`; logs en `reports/`. Para empaquetar una **nueva entrega inexistente**: `python tools/package_test.py --vpk-only`. Se rechaza reutilizar `artifacts/iterations/00.78/`. No incrementar versión por documentación sola.

## English

Requires Linux/WSL, VitaSDK, CMake, make and a C compiler. Included dependencies retain their licenses under [third_party](../third_party/README.md). Default Lua 5.1 build:

```sh
export VITASDK=/path/to/vitasdk
bash scripts/build.sh
```

Output: `build-iterations/00.78/ShotgunK2Vita-performance.vpk`. Private artwork is omitted when `private/livearea/` is absent. This default build **does not reproduce** the existing LuaJIT VPK.

Delivery 00.78 was built with `SG_LUAJIT_EXPERIMENTAL=ON`. It needs [hyln9/vita-luajit](https://github.com/hyln9/vita-luajit), reference `0f0252ab8615bc4b097da0bfe0bf10d0666a3dcd`, under `_local/vita-luajit-source-20261017/`, and [hyln9/vita-libdl](https://github.com/hyln9/vita-libdl), `bf46551882c344e5aebee07f15267e2ca93d78a5`, under `_local/vita-libdl/`, built following upstream instructions. It also links `taihen_stub`. Binary reproducibility is not guaranteed: local dependency changes have not been audited for this publication.

```sh
cmake -S . -B build-iterations/00.78 -DSG_LUAJIT_EXPERIMENTAL=ON -DCMAKE_BUILD_TYPE=Release
bash scripts/build.sh
```

PC regression with private assets: `bash scripts/check_iteration.sh`; logs under `reports/`. To package a **new delivery that does not exist yet**: `python tools/package_test.py --vpk-only`. Reusing `artifacts/iterations/00.78/` is rejected. Do not increase versions for documentation alone.

ES: El árbol de código actual conserva ajustes posteriores al empaquetado en `src/gpu_presenter.c` (espera de renderizado) y `src/graphics.c` (blit y mezcla). Esos ajustes no están incluidos en el VPK 00.78 existente y requiere una futura entrega compilada y validada. Los símbolos de la release corresponden al VPK preservado, no a una recompilación del árbol actual.

EN: The current source tree retains post-packaging changes in `src/gpu_presenter.c` (rendering wait) and `src/graphics.c` (blit and blending). Those changes are not included in the existing 00.78 VPK and requires a future built and validated delivery. Release symbols match the preserved VPK, not a rebuild of the current tree.
