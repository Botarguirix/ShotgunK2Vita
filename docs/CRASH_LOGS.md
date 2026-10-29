# DiagnÃ³sticos / Diagnostics

## EspaÃ±ol

En 00.78, los logs de sesiÃ³n se guardan en `ux0:data/shotgunk2vita/logs/`. Conserva la carpeta completa y anota versiÃ³n, acciÃ³n que produjo el fallo y configuraciÃ³n. Recupera los archivos con VitaShell y compÃ¡rtelos por un medio privado; pueden contener informaciÃ³n personal. Un core sÃ³lo puede analizarse con el ELF exacto de su compilaciÃ³n, identificado por hashes. Nunca publiques cores ni partidas.

Desde la raÃ­z del repositorio, consulta `python tools/collect_diagnostics.py --help` para recopilar archivos locales. El script no conecta automÃ¡ticamente a la consola.

## English

Steam 0.78 alpha uses `ux0:data/shotgunk2vita_steam/logs/`; the 1.5 app uses the folder below. Keep symbols and logs from each app separate.

In 00.78, session logs are stored under `ux0:data/shotgunk2vita/logs/`. Preserve the complete folder and record version, triggering action and configuration. Retrieve files with VitaShell and share privately; they may contain personal information. Only analyze a core with its exact build's ELF, identified by hashes. Never publish cores or saves.

From the repository root, use `python tools/collect_diagnostics.py --help` for local collection options. The script does not automatically connect to the console.
