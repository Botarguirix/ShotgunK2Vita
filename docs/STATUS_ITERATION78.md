# 0.78 alpha — build 00.78

ES: Rama de publicación limpia desde main, sin commits privados ni src/code_patches.h. El encabezado se genera localmente; cero parches por defecto. Dos apps: SGKV00001 (1.5) y SGKV00002 (Steam 1.623b); carpetas y replay separados. Steam instala desde data.sgr y soporta gestos touch de cursor/arrastre/clic. Las imágenes de controles son dibujos originales. La compatibilidad completa de mods y la aceptación en Vita están pendientes. Véase README e INSTALLATION para requisitos y recorrido de consola.

EN: Clean publication branch from main, excluding private commits and src/code_patches.h. Patches are generated locally; public builds use zero patches. Two apps: SGKV00001 (1.5) and SGKV00002 (Steam 1.623b), with separate userdata/replay. Steam installs data.sgr and supports touch cursor/drag/click gestures. Control maps are original drawings. Full mod compatibility and Vita acceptance remain pending. See README and INSTALLATION for requirements and console checks.


## Validación final / Final validation — 2026-10-29

- PASS: ambos VPK ARM, versión SFO 00.78; Steam SGKV00002 y 1.5 SGKV00001. Cero parches privados en el build público. / Both ARM VPKs built with zero private patches.
- PASS: scripts/check_iteration.sh completo con ASan/UBSan y datos privados 1.5. / Complete common regression with private 1.5 data.
- PASS: 29 pruebas Python de herramientas, incluyendo preparación de mods Steam y generación vacía/validación de parches. / 29 Python tool tests.
- PASS PC: instalación SGR, segundo arranque sin reinstalar, menú 600 frames, partida 1200 frames y API touch 320 frames (cursor/clic, retorno a controles físicos). Se usó una fixture SGR privada reempaquetada desde 275 recursos extraídos del usuario; los 27 scripts coinciden con los hashes originales. No es una comprobación byte por byte del SGR comercial original en esta sesión. / Private repacked SGR fixture; all original script hashes match, not a new test of the original archive bytes.
- PASS PC: Vorbis, 300 segundos simulados, starts=2, loops=3, fx=5, failures=0, drops=0. No es una prueba audible ni de hardware. / Offline audio checks only.
- Corregidos dos fallos detectados durante la limpieza: arranque Lua 5.1 sin tabla bit; redefinición de controles C: en mayúsculas que conservaba botones anteriores por error. También se actualizaron las expectativas antiguas de input_cache (48/44) y touch trasero. / Fixed plain Lua bootstrap and uppercase pad redefinition regressions; updated stale input expectations.
- WSL interrumpió pasadas temporales; se conservó evidencia y se completó la validación desde una carpeta persistente de Linux. Los datasets originales, las entregas 00.78/00.79 y la rama privada se conservan. / Interrupted attempts preserved; final checks ran from persistent storage.

Hardware pendiente: gestos en pantalla física, rendimiento/FPS, audio real, compatibilidad completa de mods y estabilidad prolongada. Los tests touch de PC no prueban el muestreo del panel Vita. No se publicó nada; push/PR/tag/prerelease requieren confirmación del usuario.

Hardware acceptance remains pending. PC touch checks do not validate the physical Vita panel. Nothing has been published; push/PR/tag/prerelease await user confirmation.
