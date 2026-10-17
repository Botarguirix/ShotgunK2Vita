# Entregas locales por iteración

Proceso de futuras entregas: [ITERATION_PROCESS.md](ITERATION_PROCESS.md).
Informe actual: [INFORME_ROADMAP_ITERACION15.md](INFORME_ROADMAP_ITERACION15.md).

| Versión | Carpeta | Estado |
|---|---|---|
| 00.02 | `artifacts/iterations/00.02/` | Bootstrap; arranque confirmado, 1–3 FPS y sin audio |
| 00.03 | `artifacts/iterations/00.03/` | Rendimiento/audio; usuario reporta 0x8010113D |
| 00.04 | `artifacts/iterations/00.04/` | Usuario reporta mejora considerable de FPS, pero crasheo; sin medición exacta aún |
| 00.05 | `artifacts/iterations/00.05/` | Logs persistentes por ejecución, traceback Lua y símbolos nativos; pendiente de prueba en Vita |
| 00.06 | `artifacts/iterations/00.06/` | Corregido delta() ausente al mantener pad; reproducción anterior y regresión en PC |
| 00.07 | `artifacts/iterations/00.07/` | Corregido trifill() ausente con stick derecho; 5.400 frames y sanitizadores en PC; pendiente en Vita |
| 00.08 | `artifacts/iterations/00.08/` | Disparo aspr y opciones corregidos; doble búfer/RAM y texto optimizado; regresiones PC; Vita pendiente |
| 00.09 | `artifacts/iterations/00.09/` | Frame Transfer: usuario confirma juego; Exit falla; integridad DMA provoca fallback CPU |
| 00.10 | `artifacts/iterations/00.10/` | Input Cache: probado, 17,80 FPS activos; fb_ev corregido en 11 |
| 00.11 | `artifacts/iterations/00.11/` | GPU confirmada: 18,36FPS; aviso blanco reportado |
| 00.12 | `artifacts/iterations/00.12/` | Gráficos/audio confirmados;22,18FPS mixtos; toque hero nil corregido en13 |
| 00.13 | `artifacts/iterations/00.13/` | Mejora informada;26,23FPS mixtos; movimiento intermitente en estudio |
| 00.14 | `artifacts/iterations/00.14/` | Controls and Shadows:controles simplificados, sombras enC y trazas de movimiento |
| 00.15 | `artifacts/iterations/00.15/` | Game Over Recovery: eliminación durante all y protección de disparo tras morir; prueba física pendiente |
| 00.26 | `artifacts/iterations/00.26/` | GPU Overlay Cleanup: elimina botones dibujados; rear touch L1/R1; regresión local |
| 00.27 | `artifacts/iterations/00.27/` | Codex Glyph Lookup: índice O(1) para fuentes bitmap; Vita pendiente |
| 00.28 | `artifacts/iterations/00.28/` | Shadow Overdraw Budget: recorte previo de sombras fuera de pantalla |
| 00.29 | `artifacts/iterations/00.29/` | Audio Presentation Budget: ruta rápida de silencio en el mezclador |
| 00.30 | `artifacts/iterations/00.30/` | Integrated Performance Candidate: combinación final para prueba física |
| 00.31 | `artifacts/iterations/00.31/` | Integrated Candidate SFO Fix: nombre corto compatible con Vita |
| 00.32 | `artifacts/iterations/00.32/` | Custom Loading Screen: pantalla propia de arranque |

Los archivos originales de 00.02 y 00.03 permanecen además en `artifacts/`.
La entrega 00.03 conserva una copia del código publicado en ese momento.
00.04 y siguientes incluyen VPK, snapshot de código y documentación, herramienta,
configuración, validación y hashes en la carpeta de su versión. Los datos
comerciales compartidos siguen en el ZIP original; no requieren otra copia
en la consola para esta corrección.

`scripts/build.sh` selecciona `build-iterations/<SG_BUILD_VERSION>/` según CMake.
`tools/package_test.py` valida el VPK antes de entregarlo y se niega a reemplazar
una carpeta de entrega existente. Para una nueva entrega, incrementar SG_BUILD_VERSION
en CMake y actualizar el estado de esta tabla. Las carpetas de compilación son
reutilizables dentro de su versión; las entregas empaquetadas se conservan.

La validación local comprueba ZIP/CRC, SFO/título/versión, dimensiones PNG,
tipo de color indexado, profundidad de 8 bits, paleta completa sin transparencia
y referencias de LiveArea. Esto no confirma instalación ni rendimiento en Vita.

## Nombres y secuencia de optimización

| Iteración | Nombre | Estado / criterio |
|---|---|---|
| 09 / 00.09 | Frame Transfer | Probada en Vita: juego confirmado; Exit resuelto en 10; DMA descartado por integridad |
| 10 / 00.10 | Input Cache | Probada en Vita: caché funciona; fb_ev corregida en 11 |
| 11 / 00.11 | GPU Presenter | Entrega actual: escalado GXM, logro local; hardware pendiente |
| 12 / 00.12 | Frame Budget | Entrega actual: composición directa, aviso local; hardware pendiente |

09 y 10 tienen entregas independientes. 12 es una etapa propuesta, adaptables a los
logs siguientes. Las carpetas históricas permanecen intactas. CMake contiene
SG_BUILD_VERSION y SG_BUILD_NAME; el paquete registra iteration.json y nombre
legible del VPK. No incrementar versión para actualizar resultados solamente.

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

16. Half Resolution and Thread Audit — divisor reversible, workers GPU y
métricas de hilos/coste gráfico; aceptación física pendiente.
