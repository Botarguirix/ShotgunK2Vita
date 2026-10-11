# VPK experimental 00.07

## Actualizar desde 00.02 / instalación fallida de 00.03

Instalar `artifacts/iterations/00.07/ShotgunK2Vita-performance.vpk` desde VitaShell sobre el mismo título
`SGKV00001`. **Conservar `ux0:data/shotgunk2vita/` y su carpeta `save/`.**
No hace falta copiar otra vez el APK, `data.sgr` ni el ZIP de datos de 00.02.
La versión 00.03 incluye el icono del EXE y portada/gate del atlas del juego.
La versión 00.04 corrige el formato de estos PNG: índice de 8 bits, paleta
de 256 colores y sin transparencia. El error 0x8010113D comunicado por el
usuario coincide con el formato RGBA incompatible encontrado en 00.03.
Las dimensiones, CRC, paleta, XML y SFO se comprueban antes de entregar el VPK.
El usuario informa mejora considerable de FPS después de esa entrega,
pero un crasheo. La versión 00.05 registra diagnósticos persistentes para
localizar la causa; todavía no se considera corregido el crasheo.
El log recuperado del runtime anterior muestra `gamepad.lua:1994: attempt to
call global 'delta' (a nil value)`. La versión 00.07 registra `delta()` junto
a `dt()` y conserva los logs de 00.05. La prueba de regresión reproduce el
fallo previo y ahora mantiene/suelta cuatro direcciones de cruceta y stick.
Falta confirmar ese arreglo en la consola; otros errores siguen siendo posibles.

Las entregas anteriores se conservan en `artifacts/iterations/00.02/` y
`00.03/`, `00.04/` y `00.05/`; la compilación nueva está en `build-iterations/00.07/`.
No desinstalar ni borrar datos/saves para aplicar esta actualización.

Si es la primera instalación, extraer `shotgunk2vita-data.zip` y copiar su
carpeta `shotgunk2vita` a `ux0:data/`. Debe existir
`ux0:data/shotgunk2vita/code.lua`, `assets/`, `code/`, `libs/` y los `.sgimg`
junto a los PNG. El ZIP se prepara sin saves de pruebas de PC.

00.07 corrige `trifill()` ausente al inspeccionar piezas con el stick derecho.
La regresión de 5.400 frames pasa en PC; falta comprobarla en Vita.
[Detalle del diagnóstico](STATUS_ITERATION07.md).

## Controles

![Controles PS Vita](images/controls-vita.png)

X confirma/mueve; círculo cancela; cuadrado es acción especial; triángulo
recarga. Cruceta/stick izquierdo navegan y seleccionan destino. Stick derecho
apunta. R dispara; L mantenido permite movimiento arriesgado. START pausa y
SELECT muestra información.

SELECT+L/R cambia páginas. SELECT+cuadrado/círculo suple L3/R3.
SELECT+triángulo muestra/oculta el medidor. START+SELECT sale.
Las combinaciones consumen el botón hasta soltarlo para evitar que al soltar
SELECT se produzca un disparo o una recarga accidental.

En los gráficos originales de PS pueden aparecer R2/L2: corresponden a R/L
físicos. La guía externa muestra el mapa adaptado. Elegir Throne, rank 1,
Solomon y START; avanzar las viñetas con X.

## Rendimiento y audio

Sin configuración, solicita CPU 444 MHz y tres participantes para presentación.
El trabajo de Lua sigue en un hilo. Audio funciona en un hilo prioritario
independiente, con MP3 por streaming y mezcla WAV PCM16 estéreo a 44,1 kHz.
Se usa el patrón de dos buffers de la implementación de música de Touhou 7.5.

El HUD muestra FPS reales entre presentaciones y tiempos U/D/P en ms.
[Herramienta de rendimiento](PERFORMANCE_TOOL.md) explica perfiles balanced,
single y battery, y la generación de informes HTML.

Las pruebas de PC alcanzan partida activa, comparan imágenes idénticas con
uno/tres hilos y verifican cinco minutos de audio mezclado, pista A→B y bucles.
Falta confirmar en Vita la mejora de FPS, sonido audible, latencia y estabilidad.
Audio OGG de la intro alternativa, crossfade completo, canales musicales
simultáneos, shaders/CRT y compatibilidad de todas las cartas/modos siguen
pendientes. Las partidas antiguas codificadas no se importan.

## Qué enviar después de probar

Después de reproducir el fallo, copiar toda la carpeta:

- `ux0:data/shotgunk2vita/logs/`
- El `.psp2dmp` reciente en `ux0:data/`, si Vita lo genera.

Las sesiones ya no se sobrescriben al volver a abrir el juego. Dentro de cada
una están `runtime.log`, `audio.log`, `performance.csv` y `crash-trace.log`.
[Guía de recuperación y ZIP de diagnóstico](CRASH_LOGS.md).

Informar FPS en título/menú/tablero, si se oye música y efectos, y si los
controles responden. Si hay cierre o pantalla de error, conservar también
el código de error y describir la última pantalla.

[Roadmap y límites de la evidencia](ROADMAP.md).
