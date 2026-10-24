# Diagnósticos / Diagnostics

## Español

En 00.78, los logs de sesión se guardan en `ux0:data/shotgunk2vita/logs/`. Conserva la carpeta completa y anota versión, acción que produjo el fallo y configuración. Recupera los archivos con VitaShell y compártelos por un medio privado; pueden contener información personal. Un core sólo puede analizarse con el ELF exacto de su compilación, identificado por hashes. Nunca publiques cores ni partidas.

Desde la raíz del repositorio, consulta `python tools/collect_diagnostics.py --help` para recopilar archivos locales. El script no conecta automáticamente a la consola.

## English

In 00.78, session logs are stored under `ux0:data/shotgunk2vita/logs/`. Preserve the complete folder and record version, triggering action and configuration. Retrieve files with VitaShell and share privately; they may contain personal information. Only analyze a core with its exact build's ELF, identified by hashes. Never publish cores or saves.

From the repository root, use `python tools/collect_diagnostics.py --help` for local collection options. The script does not automatically connect to the console.
