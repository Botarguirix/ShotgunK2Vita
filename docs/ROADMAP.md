# Roadmap · ShotgunK2Vita

Proceso de entregas: [ITERATION_PROCESS.md](ITERATION_PROCESS.md).
Informe actual: [INFORME_ROADMAP_ITERACION09.md](INFORME_ROADMAP_ITERACION09.md).

Actualizado: 2026-10-11. El usuario confirmó que 00.02 arranca en Vita OLED,
pero funciona a 1–3 FPS en todas las pantallas y sin sonido. El objetivo de
00.03 es corregir costes evitables, habilitar audio y medir la consola.
El usuario reportó después error 0x8010113D. La entrega 00.04 corrige los PNG
RGBA de LiveArea y separa las compilaciones/entregas en carpetas por versión.
Tras 00.04, el usuario informa una mejora considerable de FPS y un crasheo sin
log recuperado. No hay aún valores exactos de FPS ni confirmación de audio.
La iteración 00.05 añade logs por ejecución, checkpoints y símbolos para
diagnosticar ese fallo; su causa sigue desconocida.
Después se recibió el log del runtime anterior: `gamepad.lua:1994` llama a
`delta()` al mantener una dirección, pero faltaba registrar ese nombre.
00.06 añade el alias de `dt()`. El log muestra 21–29 FPS antes de la parada,
núcleos 0/1/2 y CPU real 500 MHz (se solicitaron 444); no es una medición de
todos los modos ni un resultado de 00.05. El rótulo `performance-00.03` era
compartido por 00.03/00.04. Desde 00.06 el SFO, log y carpeta comparten versión.
No se considera un port terminado ni se promete una tasa de FPS aún sin medir.

| Fase | Estado | Evidencia / criterio de salida |
|---|---|---|
| Identificar datos y motor | Completada | APK ARM64; scripts Lua y recursos disponibles; Steam x86 y `data.sgr` estudiados |
| Runtime y primer VPK 00.02 | Arranque confirmado en Vita | Menú y partida comprobados en PC; usuario confirma ejecución lenta en OLED |
| Optimización de dibujo 00.03 | Implementada; pruebas de PC | Cache de glifos TTF, recorte de regiones, rellenos por filas y escalado con copias; escena de regresión coincide píxel a píxel |
| Reparto entre núcleos | Implementado; pendiente en Vita | Lua permanece en un hilo; presentación repartida entre tres participantes con semáforos; imágenes de uno/tres hilos iguales en PC |
| Herramienta de rendimiento | Implementada | HUD, CSV, informe HTML y presets; siguiente paso: recibir mediciones reales de Vita |
| Diagnóstico de crasheos 00.05 | Implementado; pruebas de PC | Sesiones conservadas, checkpoints ~250 ms, traceback Lua, ELF/map con símbolos; fixture de terminación abrupta y reapertura |
| Parada al mantener pad 00.06 | Causa identificada; corrección en PC | Reproducido delta() nil en gamepad.lua:1994; registro del alias y regresión con cuatro direcciones de pad/stick |
| Música y efectos | Implementados; pendientes de escucha en Vita | MP3 por streaming, WAV PCM16, mezcla estéreo 44,1 kHz, hilo prioritario y dos buffers; prueba de 300 segundos con A→B, tres bucles y cinco efectos |
| Controles adaptados | Implementados; pendientes de recorrido en Vita | Corregidos códigos PS en mayúsculas y liberación de sticks; gatillos y páginas separados con SELECT; guía gráfica incluida |
| Icono y portada LiveArea | Formato corregido en 00.04; usuario reporta ejecución tras la entrega | PNG indexados opacos de 8 bits, paleta de 256 colores; icono de RT_ICON del EXE y portada del atlas del APK |
| Rendimiento jugable | Mejora confirmada en un log de Vita; estabilidad pendiente | 21–29 FPS antes del error del pad con CPU real 500 MHz; falta CSV de recorrido completo y 30 FPS estables |
| Compatibilidad y calidad | Pendiente | Partidas completas, cartas, jefes, otros modos, saves y texto; evitar errores de API y verificar imagen/audio en consola |

## Qué cambió en 00.03

Antes se rasterizaba una letra TTF cada vez que se dibujaba, incluso cuando
el contorno de texto la repetía muchas veces. Ahora se conserva el bitmap y
su avance por fuente/código Unicode. Los sprites se recortan antes de recorrer
píxeles; los rellenos simples usan `memset` por fila; el escalado ya no repite
divisiones para todos los píxeles de 960×544. Los workers sólo leen superficies
y paleta mientras Lua está detenido en la barrera de presentación.

La primera comparación de PC, en Release y sobre el mismo recorrido de 4.800
fotogramas (excluidos los primeros 300), pasó de 3,940 a 1,029 ms de media.
El dibujo pasó de 1,118 a 0,582 ms y la presentación de 2,542 a 0,223 ms.
Esto demuestra una reducción de trabajo en PC, no un resultado de FPS en Vita.
El CSV de la siguiente prueba permitirá determinar qué coste sigue dominando.

Validación adicional: AddressSanitizer y UndefinedBehaviorSanitizer no emitieron
diagnósticos al comprobar combinaciones y entradas, mezclar 300 segundos de
audio, comparar la escena de gráficos con tres hilos y ejecutar 4.800
fotogramas del juego hasta partida activa. El VPK y las imágenes LiveArea se
validan por separado; estas pruebas no ejercen las APIs nativas de Vita.

## Referencia de Touhou 7.5

Se revisó la conversación **Crear port de Touhou 7.5 para Vita** y la
implementación local de `vita/src/music_probe.h`. El usuario confirmó que la
música completa se escuchó sin problemas en la corrida 85. La estrategia
reutilizada aquí es salida BGM estéreo 44,1 kHz, hilo independiente prioritario,
dos buffers alineados y drenaje sólo al cerrar. El nuevo backend de Shotgun
King añade MP3 y mezcla de efectos WAV; no copia el puente DirectSound ni
ejecuta los binarios Android/Steam del juego.

## Evidencia de 00.06 y corrección 00.07

El usuario informa menores tiempos de carga. La sesión recibida se detiene
por `trifill()` ausente al inspeccionar piezas con el stick derecho. 00.07
implementa triángulos; pasan 5.400 frames en PC y sanitizadores. Antes del
error, la media de 774 frames equivale a 20,19 FPS; los últimos 120 a 14,53.
Se excluye la pantalla detenida y no se extrapola una mejora de 00.07 en Vita.
El audio no pierde bloques de cola, pero registra dos bloques tardíos y no
confirma salida audible. [Diagnóstico completo](STATUS_ITERATION07.md).

## Estado de 00.08

Los logs de 00.07 identifican aspr ausente al disparar y bget ausente en
opciones. 00.08 corrige esas rutas, añade doble framebuffer y composición
en RAM, y optimiza máscaras de texto. Pasan 6000 frames de combate en PC
y la batería ampliada con sanitizadores. La referencia activa de 00.07
es 19,53 FPS; mejora de FPS y parpadeo de 00.08 pendientes en hardware.
Persistencia, más cartas/pisos y APIs incompletas impiden declarar el port
estable. [Informe y criterios](INFORME_ROADMAP_ITERACION09.md).

## Próxima prueba en OLED

Instalar 00.08 sobre el mismo título `SGKV00001`, conservar datos/saves y hacer
el mismo recorrido con balanced y single. Para cada prueba conservar:
los archivos de cada sesión en `ux0:data/shotgunk2vita/logs/`, incluido
`crash-trace.log`, y cualquier `.psp2dmp` reciente en `ux0:data/`.
[Instrucciones para recuperar logs](CRASH_LOGS.md). Confirmar FPS de menú/tablero,
continuidad de música, efectos al navegar/disparar, controles y LiveArea.

Si domina Lua, perfilar funciones y asignaciones antes de decidir la migración
a LuaJIT. Si domina dibujo/presentación, evaluar un backend GXM/GPU conservando
las operaciones de paleta y lectura de píxeles. No añadir hilos que muten el
mismo estado Lua ni el framebuffer simultáneamente sobre las mismas filas.

## Límites actuales del audio y del port

Música MP3 por streaming y efectos WAV PCM16 están implementados. El efecto
OGG de la introducción alternativa Headbang sigue pendiente. La transición
actual sustituye pista con entrada gradual; el crossfade con dos pistas a la
vez, sincronización/avance y la semántica completa de canales musicales de
SUGAR aún requieren trabajo. Los recursos permanecen fuera de GitHub.

Texto multilínea, fidelidad de ciertas operaciones gráficas, todas las llamadas
SUGAR, importación de saves codificados, CRT y mods siguen pendientes de
compatibilidad completa. Las pruebas de PC no sustituyen pruebas de consola.

Referencias: [hilos VitaSDK](https://docs.vitasdk.org/group__SceThreadMgrUser.html),
[formato PNG LiveArea y error 0x8010113D](https://gist.github.com/hammerill/64411eebf071b93396b7d310ba8d6776),
[dr_mp3](https://github.com/mackron/dr_libs/blob/master/dr_mp3.h),
[API pública SUGAR](https://github.com/TRASEVOL-DOG/Shotgun-King-Modding-Guide).

## Estado actual: Iteración 09 — Frame Transfer

El usuario confirma gran mejora de 00.08. Su log contiene 7036 frames activos:
18,00 FPS derivados, media 55,56 ms y P95 83,42 ms. Update 10,96 ms, draw
19,86 ms, presentación 6,14 ms, copia 8,41 ms y espera 7,75 ms. Tres
participantes, reloj observado 500 MHz. Son fases distintas de las corridas
anteriores: no demostrar una ganancia o regresión con medias incomparables.
La sesión termina por ismobile() ausente en la pausa. 09 registra esa API y convierte los índices OPTIONS[id] al puente SET,
retira HUD y compara CPU/DMA con comprobación de integridad y fallback.
La mejora de FPS de 09 y comportamiento DMA requieren prueba física.

Secuencia nombrada: 10 Input Cache, 11 GPU Presenter, 12 Frame Budget.
[Investigación de otros ports y aplicación](PORT_OPTIMIZATION_RESEARCH.md).
[Seguimiento por iteración](ITERATIONS.md).
