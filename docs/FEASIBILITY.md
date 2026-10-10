# Evaluación de archivos recibidos — 2026-10-10

## Resultado

El APK examinado no permite un port binario directo mediante so-loader: todas
sus bibliotecas son ELF64 AArch64. Vita requiere código ARM de 32 bits.
Cambiar el empaquetado o la extensión no convierte la arquitectura.

APK SHA-256: `95e56bf8f38b4cf919ccfc0d237747139dfc1b3b928575db23db0db8b57f7d2b`.
Se analizaron cabeceras, nombres, scripts y símbolos sin ejecutar el APK.

## Android

Ocho bibliotecas, todas bajo `lib/arm64-v8a`: libsugar, libfmod, libEOSSDK,
libc++_shared, libgpdeku, libpairipcore, libstub y libAPKVISION.
El análisis no asume que este paquete modificado coincida con una distribución
oficial. No se ha intentado desactivar sus protecciones o ejecutar sus añadidos.

`libsugar.so` depende de EGL, GLESv3, jnigraphics, android, log, EOSSDK, FMOD,
libm, libc++, libdl y libc. Exporta JNI_OnLoad, luaL_newstate y métodos JNI de
MainActivity para Tick, Resume, Suspend, plataforma y EOS. Incluso una edición
ARM32 necesitaría adaptación de gráficos, audio y JNI; no bastaría con cargarla.

Hay 494 entradas de archivos en assets, incluidos 34 scripts Lua, 200 PNG,
155 WAV, 21 MP3, 16 OGG y 8 shaders. `assets/code.lua` define `_init` y carga
scripts auxiliares y recursos mediante la API SUGAR (newsrf, newwin, palette,
controls, addfont, etc.). Esto abre una ruta de runtime compatible, pero no
demuestra que los scripts funcionen en un intérprete Lua genérico.

`assets/sugar.sgr` comienza con una entrada nombrada discord_game_sdk_dll y
contenido PE; no debe confundirse con el motor Android compilado.

## Steam

`data.sgr`: 82,821,343 bytes, no es un ZIP reconocido. Formato aún sin
identificar; no se afirma que esté cifrado. La carpeta contiene
shotgun_king.exe, SDL3.dll, steam_api.dll y discord_game_sdk.dll. No se encontró
un proyecto fuente del motor en el nivel superior de esta instalación.

## Rutas viables por investigar

1. Obtener de una distribución legítima una compilación real `armeabi-v7a`
   de SUGAR y sus dependencias. No se ha confirmado que exista. Con ella se
   vuelve a evaluar ABI, imports, JNI y GLES antes de implementar so-loader.
2. Acceso al código fuente de SUGAR para recompilar y adaptar el motor a Vita.
   No se localizó un repositorio público del motor en las búsquedas realizadas.
3. Implementar un runtime compatible con la API que usan estos scripts,
   empezando por lectura local, Lua, superficies, paleta, entrada y texto;
   después menú, gameplay, sonido y guardado. Es un desarrollo considerable.

[Sugarcoat](https://github.com/TRASEVOL-DOG/sugarcoat), del autor de SUGAR,
reproduce parte de su comportamiento en LÖVE/Castle; es una referencia técnica,
no un runtime Vita listo ni una prueba de compatibilidad con Shotgun King.
Revisar permisos/licencia antes de incorporar código de esa referencia.
La [guía oficial de modding](https://github.com/TRASEVOL-DOG/Shotgun-King-Modding-Guide)
documenta el motor y enlaza el manual de su API.

## Estado del entregable

El VPK continúa siendo diagnóstico nativo. No incorpora ni ejecuta el juego.
Los archivos comerciales y los informes con rutas locales permanecen excluidos
de Git. El siguiente hito de la ruta compatible sería ejecutar `_init` con un
registro preciso de las funciones SUGAR pendientes, antes de prometer gameplay.
