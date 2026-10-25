# Informe / Progress · 00.78

ES: Documentación reorganizada, versión unificada e instalación y herramientas en español e inglés. Entrega local intacta. Siguiente prueba: aceptación de consola, saves y medición por escena. [Estado](STATUS_ITERATION78.md) · [Roadmap](ROADMAP.md).

EN: Documentation reorganized, version references unified, installation and tools in Spanish and English. Local delivery intact. Next check: console acceptance, saves and per-scene measurements. [Status](STATUS_ITERATION78.md) · [Roadmap](ROADMAP.md).

### Validación de publicación / Publication validation

ES: ZIP/SFO/LiveArea y hashes VPK/ELF/map correctos; extracción sintética y ayuda de las cinco herramientas correctas. [GitHub Actions](https://github.com/Botarguirix/ShotgunK2Vita/actions/runs/40065075109) compiló el árbol con Lua 5.1 predeterminado. Esto no recompila el VPK LuaJIT preservado. La regresión común ASan/UBSan pasó iteradores, reinicio tras Game Over y copia de fotogramas; se detuvo en `sg_inputs_probe`, cuyas aserciones esperan las zonas traseras antiguas (incluido L1 en la esquina 0,0). El código actual mantiene márgenes exteriores inactivos. El resto de la regresión no se ejecutó tras ese fallo. Hardware pendiente.

EN: ZIP/SFO/LiveArea and VPK/ELF/map hashes passed; synthetic extraction and all five tool help commands passed. [GitHub Actions](https://github.com/Botarguirix/ShotgunK2Vita/actions/runs/40065075109) built the default Lua 5.1 source tree. This does not rebuild the preserved LuaJIT VPK. The ASan/UBSan common regression passed mutation-safe iteration, Game Over restart and frame copying; it stopped at `sg_inputs_probe`, whose assertions expect obsolete rear-touch zones (including L1 at corner 0,0). Current code intentionally leaves outer margins inactive. Remaining regression checks did not run after that failure. Hardware pending.


## Investigación Steam SGR / Steam SGR investigation

ES: La rama `codex/steam-sgr-feasibility` evalúa datos preparados exclusivamente desde `data.sgr` con C/LuaJIT. Hay pruebas PC de menú, partida y audio; no hay un VPK Steam ni aceptación en consola. Conserva intacta la entrega 00.78. [Pasadas, herramientas y límites](STEAM_SGR_FEASIBILITY.md).

EN: Branch `codex/steam-sgr-feasibility` evaluates data prepared exclusively from `data.sgr` with C/LuaJIT. PC menu, game and audio checks exist; no Steam VPK or console acceptance is provided. Delivery 00.78 remains intact. [Passes, tools and limits](STEAM_SGR_FEASIBILITY.md#english).
