# Rama experimental LuaJIT PS Vita

Fuente de referencia: `hyln9/vita-luajit`, conservada fuera del runtime estable
en `_local/vita-luajit-source-20261017/`. Su README requiere compilar con
`TARGET_SYS=PSP2`, `arm-vita-eabi` y `vita-libdl` para FFI.

Las entregas 00.33–00.37 son cinco puertas de compatibilidad. La macro
`SG_LUAJIT_EXPERIMENTAL` identifica la rama, pero el intérprete estable sigue
siendo Lua 5.1 hasta que una etapa demuestre que puede ejecutar los scripts
SUGAR sin cambiar semántica. No se afirma una mejora de FPS antes de medirla.

| Versión | Puerta | Criterio |
|---|---|---|
| 00.33 | Build Probe | VitaSDK y VPK válidos |
| 00.34 | API Compatibility | registros C y bootstrap sin error |
| 00.35 | Script Bootstrap | `code.lua`, menú y Codex |
| 00.36 | Runtime Benchmark | CPU, memoria, draw y FPS comparables |
| 00.37 | Vita Candidate | instalación, Vita3K y hardware pendiente |
