# Instalación y controles / Installation and controls

## Español

Requiere PS Vita con homebrew y VitaShell, el VPK 00.78 y datos preparados desde un APK compatible de tu propia copia. No se ha establecido una matriz completa de firmware y plugins para esta entrega.

1. Descarga el VPK de [Releases](https://github.com/Botarguirix/ShotgunK2Vita/releases).
2. Para la primera instalación, sigue [DATA_TOOLS](DATA_TOOLS.md) y copia el **contenido** de `private/game/` a `ux0:data/shotgunk2vita/`.
3. Comprueba que exista `ux0:data/shotgunk2vita/code.lua`, las subcarpetas `assets/` y los `.sgimg` generados. Copiar sólo `data.sgr` no funciona.
4. Instala el VPK con VitaShell. Para actualizar conserva datos, configuración y `save/`; no copies un `save/` vacío encima del existente.
5. La configuración de rendimiento de la release es opcional. Respalda tu `performance.cfg` antes de sustituirlo. Un límite de 60 FPS no garantiza alcanzar 60 FPS.

| Control | Acción |
|---|---|
| Cruceta / stick izquierdo | Seleccionar destino |
| × | Confirmar movimiento / selección |
| Stick derecho | Apuntar |
| R / L | Disparar / acción arriesgada |
| □ / △ / ○ | Especial / recargar / cancelar |
| START / SELECT | Pausa / información |
| Panel trasero, zona superior izquierda / derecha | L1 / R1, cambiar páginas |

El táctil frontal está desactivado. Sal desde el menú; si aparece la pantalla de error del runtime, START permite salir. Los diagnósticos quedan en `ux0:data/shotgunk2vita/logs/`; compártelos de forma privada si contienen información personal.

Recorrido de verificación: abrir menú, iniciar partida, mover y confirmar con ×, apuntar/disparar/recargar, probar páginas traseras, pausar, perder y reintentar, salir y volver a abrir para comprobar los saves. La aceptación en consola de 00.78 sigue pendiente.

## English

Requires a homebrew-enabled PS Vita with VitaShell, the 00.78 VPK and data prepared from a compatible APK of your own game copy. A complete firmware and plugin compatibility matrix has not been established for this delivery.

1. Download the VPK from [Releases](https://github.com/Botarguirix/ShotgunK2Vita/releases).
2. For a first installation, follow [DATA_TOOLS](DATA_TOOLS.md#english) and copy the **contents** of `private/game/` to `ux0:data/shotgunk2vita/`.
3. Check for `code.lua`, the `assets/` subdirectories and generated `.sgimg` files. Copying only `data.sgr` does not work.
4. Install using VitaShell. When updating preserve data, settings and `save/`; never overwrite saves with an empty `save/` directory.
5. The release performance configuration is optional. Back up `performance.cfg` before replacing it. A 60 FPS limit does not guarantee 60 FPS performance.

| Control | Action |
|---|---|
| D-pad / left stick | Select destination |
| × | Confirm movement / selection |
| Right stick | Aim |
| R / L | Shoot / risky action |
| □ / △ / ○ | Special / reload / cancel |
| START / SELECT | Pause / information |
| Rear touch, upper left / upper right | L1 / R1, change pages |

Front touch is disabled. Exit through the menu; on the runtime error screen press START to exit. Diagnostics stay in `ux0:data/shotgunk2vita/logs/`; share privately if they contain personal information.

Console check: open menu, start a game, move and confirm with ×, aim/shoot/reload, switch rear-touch pages, pause, lose and retry, exit and reopen to check saves. Hardware acceptance for 00.78 remains pending.


## Investigación Steam SGR / Steam SGR investigation

ES: La rama `codex/steam-sgr-feasibility` evalúa datos preparados exclusivamente desde `data.sgr` con C/LuaJIT. Hay pruebas PC de menú, partida y audio; no hay un VPK Steam ni aceptación en consola. Conserva intacta la entrega 00.78. [Pasadas, herramientas y límites](STEAM_SGR_FEASIBILITY.md).

EN: Branch `codex/steam-sgr-feasibility` evaluates data prepared exclusively from `data.sgr` with C/LuaJIT. PC menu, game and audio checks exist; no Steam VPK or console acceptance is provided. Delivery 00.78 remains intact. [Passes, tools and limits](STEAM_SGR_FEASIBILITY.md#english).
