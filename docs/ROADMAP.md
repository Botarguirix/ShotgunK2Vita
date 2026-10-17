# Roadmap · ShotgunK2Vita

Estado actual:15 Game Over Recovery. El error tras morir de v14 fue reproducido
en PC: all omitía controles al eliminarlos. Corrección del iterador y protección
del disparo tardío; aceptación física15 pendiente. Mantiene controles14 y sombras
enC. Desajuste de sprite todavía en seguimiento, sin causa completa demostrada.

2026-10-13: [rutina de optimización por pases](OPTIMIZATION_ROUTINE.md) añadida.
Regresión PC como barrera, mediciones Release por escena y comparación de
huellas/variación antes de aceptar candidatos. Vita3K tiene arranque confirmado;
entrada interactiva y aceptación final en consola pendientes.


Proceso de entregas: [ITERATION_PROCESS.md](ITERATION_PROCESS.md).
Informe actual: [INFORME_ROADMAP_ITERACION15.md](INFORME_ROADMAP_ITERACION15.md).

Actualizado: 2026-10-12. El usuario confirmó que 00.02 arranca en Vita OLED,
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
estable. [Informe y criterios](INFORME_ROADMAP_ITERACION11.md).

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

## Estado actual: Iteración 10 — Input Cache

Usuario: 00.09 sin otros errores de juego; Exit falló por quit() ausente.
10168 muestras activas: 18,38 FPS derivados, media 54,42 ms y P95 83,41 ms.
Update 13,29 ms, draw 20,18 ms, present 6,09 ms, copy 5,54 ms y wait 7,10 ms.
El autotest DMA falla integridad en el pase 6 con rc=0 y elige CPU; conservar
esa protección. Este recorrido no demuestra por sí solo mejora sobre otra sesión.
10 elimina parseo repetido, memoiza entradas por frame/reasignación y añade
quit como solicitud de teardown nativo. La prueba de controles PC conserva
semántica y reduce el trabajo medido; FPS en Vita pendientes.
Siguiente 11 GPU Presenter: draw aún domina, estudiar composición/escalado GPU
con imagen equivalente y sin parpadeo. 12 Frame Budget: perfilar primitivas/Lua,
media/P95 y sesiones largas. No volver a habilitar un HUD propio.

10 incorpora además L1/R1 táctiles; consumo de sus toques y alternativa SELECT+L/R.

## Actualización 00.11 — GPU Presenter

1756 muestras activas: 17,80 FPS derivados, frame medio 56,178 ms y P95
83,372 ms. Update 9,248; draw 19,984; present 6,249; copy 8,598;
wait 9,378 ms. CPU observado 500 MHz, tres participantes. Sólo un parseo de
entrada: la caché funciona. Son recorridos distintos; no inferir regresión
respecto a la media de 00.09. DMA rc=0 falla integridad: fallback CPU correcto.

Error real: code/achievements.lua:455 llama fb_ev() inexistente después de
asignar DEN.achievements[id]. No falta el desbloqueo, falta la API de analítica.

GPU de 320×180 implementada; regresiones PC/guardado y compilación Vita válidas.
Hardware pendiente. Próxima 12 Frame Budget: medir GXM y perfilar dibujo/Lua.
Detalles: [STATUS_ITERATION11.md](STATUS_ITERATION11.md).

## Actualización 00.12 — Frame Budget

6089 muestras activas: 18,36 FPS derivados; media54,479ms, P95 83,382ms.
Update9,959ms, draw19,240ms, present12,577ms, copy/subida0,639ms,
envío GXM0,226ms, espera9,339ms. CPU observado500MHz. Exit termina limpio,
sin traceback Lua. El usuario reporta aviso de logro blanco. El título puede
rondar55FPS en tramos del log, pero la partida contiene caídas a14–18FPS;
no es 30/60 estable. Transferencia pequeña funciona; composición aún costosa.

12 optimiza composición, retira enlaces y sustituye aviso conservando logros.
13 Draw Budget perfilará dibujo/Lua. [Detalles](STATUS_ITERATION12.md).

## Actualización00.13 — Draw Budget

Usuario confirma gráficos y todos los sonidos correctos; menú aproximadamente
30FPS con algunas caídas. CSV:2008 muestras activas antes del fallo,22,18FPS
promedio mixto,45,087ms/frame y P95 66,742ms. Update11,346; draw18,879;
present1,745; subida0,638; envíoGXM0,227; espera10,244ms. CPU observado500MHz.
No incluir la pantalla detenida ni comparar directamente recorridos distintos.
La reducción de composición lógica está reflejada en estos tiempos; lógica y
dibujo (~30,23ms) dominan ahora el trabajo. No es30FPS estable en toda la partida.

13 corrige hero nil táctil y agrupa pal_rst, con contadores de entidades/sprites.
14 Entity Profile estudiará rutas dominantes. [Detalles](STATUS_ITERATION13.md).

## Actualización00.14 — Controls and Shadows

28727 muestras activas:26,23FPS mixtos,38,120ms/frame y P95 50,081ms.
Update7,213ms, draw13,675, present1,721, subida0,609, envíoGXM0,223,
espera13,145. CPU observado500MHz. Cierre por Exit, sin error Lua en esta sesión.
Entidades raíz media78,2/máximo279; callbacks de dibujo86,4/máximo326.
Correlación con draw_ms:entidades0,494, callbacks0,754, sprites0,719; no prueba
causalidad. No basta para garantizar60FPS en juego:presupuesto16,7ms.
El log registra82 cambios a Android. Se estudia ese cambio, sin atribuirle por
sí solo el fallo lógico/visual que describe el usuario.

14 cumple controles simplificados y sombras agrupadas; movimiento/animación
física pendiente de diagnóstico.15 Lua Budget perfilará lo restante.
[Detalles](STATUS_ITERATION14.md).

## Actualización 00.16 — Half Resolution and Thread Audit

Divisor de salida reversible, workers GPU configurables y auditoría de afinidad,
espera y coste gráfico. Vita3K y PC son preparación; FPS y estabilidad se
aceptan únicamente con una prueba de consola.

## Actualización 00.27 — Codex Glyph Lookup

Las fuentes bitmap usan ahora un índice directo de glifo para evitar el barrido
lineal por carácter en Codex y otras pantallas de texto. La salida nativa y los
controles se conservan. La medición equivalente en Vita3K y OLED sigue pendiente.
00.28 medirá overdraw/sombras; 00.29 audio/presentación; 00.30 comparará las
variantes y elegirá la candidata física.

## Actualización 00.28 — Shadow Overdraw Budget

La región de `shpr` se recorta antes de muestrear la máscara cuando queda fuera
del framebuffer. Se mantienen las pruebas de equivalencia para cámara, clip,
paleta y máscaras modificadas.

## Actualización 00.29 — Audio Presentation Budget

El mezclador evita recorrer frames y canales durante silencio total después de
procesar comandos. La ruta con música o efectos permanece sin cambios; 00.30
hará la comparación final de estabilidad y costes.

## Actualización 00.30 — Integrated Performance Candidate

Se integran 00.26–00.29 en una entrega final reversible. 00.30 es la candidata
recomendada para la OLED; las carpetas anteriores se conservan como rollback.

## Actualización 00.38 — Centered Rear Touch and Stick Guard

La nueva pasada reduce activaciones accidentales del touch trasero y filtra el drift del stick. Se mantiene el tablero sin botones dibujados.
