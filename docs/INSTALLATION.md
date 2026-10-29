# Instalación / Installation — 0.78 alpha

## Español

Steam: instala ShotgunKing2Vita-steam.vpk (SGKV00002) y copia únicamente tu data.sgr de The Final Checkmate 1.623b a ux0:data/shotgunk2vita_steam/. La app prepara los datos al primer arranque. Deja espacio libre para el archivo y su extracción. No apagues durante la preparación; si falla, conserva el original y los diagnósticos para revisar la causa.

1.5: instala ShotgunKing2Vita-performance.vpk (SGKV00001), prepara tu APK compatible con tools/prepare_data.py y copia el contenido preparado a ux0:data/shotgunk2vita/. data.sgr no sirve para esta app. Nunca mezcles ambas carpetas.

Actualiza solo el VPK conservando datos y save/. Respalda partidas, mods y performance.cfg. Steam incluye defaults en app0:performance.cfg; un performance.cfg en su carpeta de datos tiene prioridad. front_touch=1 habilita el ratón táctil; lua_jit=0 mantiene el intérprete LuaJIT; profile_funcs/profile_lines habilitan perfiles detallados; input_record/input_replay controlan grabación/replay, con prioridad de replay. Cada app usa su propia carpeta replay/.

Prueba en consola: primer arranque desde SGR, menú, Throne, mover/confirmar, apuntar/disparar/recargar, gestos touch, páginas traseras, pausa, perder/reintentar y guardar/reabrir. Prueba primero sin mods y añade uno cada vez. Hardware pendiente.

## English

Steam: install ShotgunKing2Vita-steam.vpk (SGKV00002) and copy only your The Final Checkmate 1.623b data.sgr into ux0:data/shotgunk2vita_steam/. First boot prepares resources. Keep enough space for the archive and extracted files. Do not interrupt preparation; retain the original and diagnostics if it fails.

1.5: install ShotgunKing2Vita-performance.vpk (SGKV00001), prepare a compatible user-owned APK with tools/prepare_data.py and copy the prepared contents into ux0:data/shotgunk2vita/. This app does not use data.sgr. Never mix folders.

Update only the VPK, preserving assets/save/. Back up saves, mods and performance.cfg. Steam defaults ship in app0:performance.cfg; a userdata performance.cfg overrides them. front_touch=1 enables touch mouse; lua_jit=0 uses the LuaJIT interpreter; profile_funcs/profile_lines enable detailed profiles; input_record/input_replay control recording/replay, with replay taking precedence. Each app uses its own replay/ folder.

Console checklist: first SGR boot, menu, Throne, move/confirm, aim/shoot/reload, touch gestures, rear pages, pause, defeat/retry and save/reopen. Establish a mod-free baseline first. Hardware acceptance pending.
