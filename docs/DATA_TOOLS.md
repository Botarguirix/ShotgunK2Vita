# Datos y herramientas / Data and tools

## EspaÃ±ol

InstalaciÃ³n Steam normal: solo VPK + data.sgr 1.623b; la app instala y convierte localmente las imÃ¡genes y reproduce Vorbis sin FFmpeg. No adjuntes los datos extraÃ­dos a Git o releases.

Herramientas de desarrollo: extract_sgr.py inspecciona/extrae SGR; prepare_steam_data.py prepara recursos en PC (Pillow y FFmpeg cuando convierte audio a MP3); prepare_data.py prepara APK 1.5; inspect_apk.py inspecciona el paquete; prepare_mod.py prepara mods en PC (`--game steam` conserva script.lua; `--game 1.5` permite script1.5.lua). El flujo directo de Workshop en Vita no requiere prepare_mod.py ni elegir script1.5.lua. Los scripts no incluyen recursos del juego. VÃ©ase SGR_FORMAT.md y BUILDING.md.

## English

Normal Steam installation needs only the VPK and your 1.623b data.sgr. The app extracts resources, converts images locally and plays Vorbis without FFmpeg. Do not commit or attach extracted data to releases.

Developer tools: extract_sgr.py inspects/extracts SGR; prepare_steam_data.py prepares PC resources (Pillow and FFmpeg when converting music to MP3); prepare_data.py prepares a compatible 1.5 APK; inspect_apk.py inspects that package; prepare_mod.py prepares PC mods (`--game steam` keeps script.lua; `--game 1.5` permits script1.5.lua). Direct Vita Workshop installation does not require prepare_mod.py or script1.5.lua selection. These scripts contain no game assets. See SGR_FORMAT.md and BUILDING.md.
