# Shotgun King Vita+ · ShotgunK2Vita

[Español](#español) · [English](#english)

## Español

**Shotgun King Vita+** es un proyecto homebrew experimental que adapta Shotgun King a PlayStation Vita mediante un runtime propio en C y una implementación parcial de la API SUGAR. Ejecuta scripts Lua y recursos preparados desde una copia del usuario; no carga los binarios originales de Android o Windows.

**Versión actual: 00.78 — Shotgun King Vita+** · ID: `SGKV00001`.

- [Descargas y notas de publicación](https://github.com/Botarguirix/ShotgunK2Vita/releases)
- [Instalación, actualización y controles](docs/INSTALLATION.md)
- [Preparación de datos y herramientas](docs/DATA_TOOLS.md)
- [Compilación y dependencias](docs/BUILDING.md)
- [Estado y limitaciones](docs/STATUS_ITERATION78.md) · [Roadmap](docs/ROADMAP.md)

El VPK no incluye los datos necesarios para jugar. El extractor prepara recursos de un **APK compatible suministrado por el usuario**; no convierte una compra de Steam ni extrae `data.sgr`. Consulta los requisitos antes de instalar.

La entrega existente 00.78 utiliza LuaJIT y presentación GPU. Sigue siendo experimental: no se garantiza compatibilidad con todos los modos, cartas, efectos, audio o partidas de otras versiones. No se afirma una mejora de FPS ni aceptación en consola de 00.78 sin evidencia de hardware.

Al actualizar, instala el VPK con VitaShell sobre la aplicación existente y conserva `ux0:data/shotgunk2vita/`, especialmente `save/`. No desinstales ni reemplaces tus partidas.

## English

**Shotgun King Vita+** is an experimental homebrew project adapting Shotgun King to PlayStation Vita through a custom C runtime and a partial implementation of the SUGAR API. It runs Lua scripts and assets prepared from the user's own game files; it does not load the original Android or Windows binaries.

**Current version: 00.78 — Shotgun King Vita+** · Title ID: `SGKV00001`.

- [Downloads and release notes](https://github.com/Botarguirix/ShotgunK2Vita/releases)
- [Installation, updates and controls](docs/INSTALLATION.md#english)
- [Data preparation and tools](docs/DATA_TOOLS.md#english)
- [Building and dependencies](docs/BUILDING.md#english)
- [Status and limitations](docs/STATUS_ITERATION78.md) · [Roadmap](docs/ROADMAP.md)

The VPK does not include the game data required to play. The extractor prepares assets from a **compatible APK supplied by the user**; it cannot convert a Steam purchase or extract `data.sgr`. Read the requirements before installing.

The existing 00.78 delivery uses LuaJIT and GPU presentation. It remains experimental: compatibility with every mode, card, effect, audio feature or save from other versions is not guaranteed. No FPS improvement or console acceptance is claimed for 00.78 without hardware evidence.

To update, install the VPK over the existing application with VitaShell and preserve `ux0:data/shotgunk2vita/`, especially `save/`. Do not uninstall or replace your saves.

### Créditos / Credits

Proyecto comunitario no oficial, sin afiliación con los creadores del juego ni Sony. Shotgun King y sus recursos pertenecen a sus respectivos titulares. / Unofficial community project, not affiliated with the game's creators or Sony. Shotgun King and its assets belong to their respective rights holders.

Desarrollado con asistencia de ChatGPT/Codex. / Developed with ChatGPT/Codex assistance.

Referencias y licencias / References and licenses: [SUGAR modding guide](https://github.com/TRASEVOL-DOG/Shotgun-King-Modding-Guide), [third-party notices](third_party/README.md). Las herramientas no incluyen datos extraídos del juego. / Tools contain no extracted game data.

ES: El árbol de código actual conserva ajustes posteriores al empaquetado en `src/gpu_presenter.c` (espera de renderizado) y `src/graphics.c` (blit y mezcla). Esos ajustes no están incluidos en el VPK 00.78 existente y requiere una futura entrega compilada y validada. Los símbolos de la release corresponden al VPK preservado, no a una recompilación del árbol actual.

EN: The current source tree retains post-packaging changes in `src/gpu_presenter.c` (rendering wait) and `src/graphics.c` (blit and blending). Those changes are not included in the existing 00.78 VPK and requires a future built and validated delivery. Release symbols match the preserved VPK, not a rebuild of the current tree.
