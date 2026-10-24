# Estado / Status · 00.78 — Shotgun King Vita+

## Español

Publicación de la entrega local existente, sin modificar su VPK ni sobrescribir su carpeta. El README mezclaba versiones y una interfaz obsoleta de LiveArea; se reemplaza por instrucciones actuales en español e inglés. El código local incluye ajustes de controles, gráficos y presentación GPU posteriores al código público anterior.

La entrega original conserva VPK, ELF/map, snapshot, validación y hashes en `artifacts/iterations/00.78/`. Utilizar esos símbolos sólo con ese VPK tras comprobar hashes. La documentación editorial posterior no forma parte del snapshot original.

Hardware: **pendiente**. Los resultados de integridad y regresión de esta sesión se registran por separado. ZIP/SFO/LiveArea no prueban jugabilidad, audio ni FPS; no se atribuyen resultados antiguos a 00.78.

Aceptación: seguir [INSTALLATION](INSTALLATION.md) conservando datos y saves. Si falla, aportar versión, acción y diagnósticos de forma privada.

## English

Publication of the existing local delivery without changing its VPK or overwriting its directory. The README mixed versions and an obsolete LiveArea interface; it is replaced with current Spanish and English instructions. Local code includes controls, graphics and GPU presentation changes after the earlier public code.

The original delivery retains VPK, ELF/map, snapshot, validation and hashes under `artifacts/iterations/00.78/`. Use these symbols only with that VPK after checking hashes. Later editorial documentation is not part of the original snapshot.

Hardware: **pending**. Integrity and regression outcomes for this session are recorded separately. ZIP/SFO/LiveArea checks do not prove gameplay, audio or FPS; earlier results are not attributed to 00.78.

Acceptance: follow [INSTALLATION](INSTALLATION.md#english), preserving data and saves. If it fails, provide version, triggering action and diagnostics privately.

ES: El árbol de código actual conserva ajustes posteriores al empaquetado en `src/gpu_presenter.c` (espera de renderizado) y `src/graphics.c` (blit y mezcla). Esos ajustes no están incluidos en el VPK 00.78 existente y requiere una futura entrega compilada y validada. Los símbolos de la release corresponden al VPK preservado, no a una recompilación del árbol actual.

EN: The current source tree retains post-packaging changes in `src/gpu_presenter.c` (rendering wait) and `src/graphics.c` (blit and blending). Those changes are not included in the existing 00.78 VPK and requires a future built and validated delivery. Release symbols match the preserved VPK, not a rebuild of the current tree.

### Validación de publicación / Publication validation

ES: ZIP/SFO/LiveArea y hashes VPK/ELF/map correctos; extracción sintética y ayuda de las cinco herramientas correctas. [GitHub Actions](https://github.com/Botarguirix/ShotgunK2Vita/actions/runs/40065075109) compiló el árbol con Lua 5.1 predeterminado. Esto no recompila el VPK LuaJIT preservado. La regresión común ASan/UBSan pasó iteradores, reinicio tras Game Over y copia de fotogramas; se detuvo en `sg_inputs_probe`, cuyas aserciones esperan las zonas traseras antiguas (incluido L1 en la esquina 0,0). El código actual mantiene márgenes exteriores inactivos. El resto de la regresión no se ejecutó tras ese fallo. Hardware pendiente.

EN: ZIP/SFO/LiveArea and VPK/ELF/map hashes passed; synthetic extraction and all five tool help commands passed. [GitHub Actions](https://github.com/Botarguirix/ShotgunK2Vita/actions/runs/40065075109) built the default Lua 5.1 source tree. This does not rebuild the preserved LuaJIT VPK. The ASan/UBSan common regression passed mutation-safe iteration, Game Over restart and frame copying; it stopped at `sg_inputs_probe`, whose assertions expect obsolete rear-touch zones (including L1 at corner 0,0). Current code intentionally leaves outer margins inactive. Remaining regression checks did not run after that failure. Hardware pending.
