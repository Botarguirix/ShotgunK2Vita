# Iteración 13 — Draw Budget · 00.13

## Resultado recibido: Vita00.12

Entrada privada 00.12-1791778390-0 preservada en reports/iterations/00.13/input.
Usuario confirma gráficos y todos los sonidos correctos; menú aproximadamente
30FPS con algunas caídas. CSV:2008 muestras activas antes del fallo,22,18FPS
promedio mixto,45,087ms/frame y P95 66,742ms. Update11,346; draw18,879;
present1,745; subida0,638; envíoGXM0,227; espera10,244ms. CPU observado500MHz.
No incluir la pantalla detenida ni comparar directamente recorridos distintos.
La reducción de composición lógica está reflejada en estos tiempos; lógica y
dibujo (~30,23ms) dominan ahora el trabajo. No es30FPS estable en toda la partida.

## Error y corrección

El log cambia a touches/Android antes de code.lua:13890: hero es nil en
mk_sq_but.dr, que consulta hero.special. Es error Lua controlado, no evidencia
de agotamiento de memoria ni crasheo nativo. La casilla se dibuja sin rey presente.
El cargador añade una comprobación hero and en esa expresión concreta, en
memoria. Conserva datos originales, líneas del traceback, controles físicos,
toque de tablero y L1/R1. Registra vita_touch_hero_guard=applied o
pattern_not_found; se dirige al paquete conocido y no cubre cualquier versión.
No crea un rey artificial ni descarta entidades para evitar el error.

Prueba con bibliotecas12 reproduce la misma línea y error. En13 la casilla sin
hero y varios frames con toque/Android continúan sin error, con ASan/UBSan.

## Entidades y dibujo

Hipótesis del usuario: entidades/casillas elevan el coste. Se encontró que dre
llama pal_rst por entidad:20 llamadas de pal,10 de palt y1 de sfillp, además de
bucles Lua.13 ejecuta el mismo reset mediante una llamada C, preservando sólo
los índices0–9 y el patrón de color1. Paletas especiales y otros índices no se
reinician. Si pal ha sido reemplazado por Lua (pausa/modificaciones), utiliza
sus llamadas originales; no asume que sea un no-op. No paraleliza Lua.

Equivalencia de imagen frente al reset de referencia, normal, pausa y pal Lua
que delega: hashes78b958c5 /6f580cc5 /78b958c5. Ensayo PC de60000 resets:
344768→15140µs (~22,8× para esa operación). No es ganancia de FPS del juego ni
medición ARM. Pruebas comunes de6000 frames de combate, pausa, Exit, logros,
Códice, opciones, pad, presentación y diagnósticos pasan con sanitizadores.
El fallback de paleta final se valida además por prueba específica de tres casos.

CSV añade entities (lista raíz), entity_updates, entity_draws (llamadas,
incluidas entidades que luego omiten dibujo), palette_resets,
palette_suppressed (ruta pal reemplazada) y sprite_calls. blit_pixels ya existía.
Counts de dibujo pueden superar lista raíz por hijos/otras listas. Los wrappers
conservan argumentos/resultados y sólo incrementan contadores. Prueba real de
título:6 entidades,7 updates,9 llamadas de dibujo/reset y7 sprites.
No medir estas cifras de PC como utilización ni número de casillas en Vita.

La herramienta muestra media/máximo y correlación con draw_ms. No demuestra
causalidad; comparar misma escena. En12 sólo existía blit_pixels: correlación
mixta -0,241 con draw_ms, insuficiente para atribuir el coste al número de
píxeles/casillas. La lista de entidades y el coste por callback requieren logs13.
Se conservan todas las entidades, reglas del tablero, sprites y efectos.

## Entrega y prueba

VitaSDK Release compilado; SFO00.13, ZIP/CRC y LiveArea validados.
Paquete independiente artifacts/iterations/00.13/
ShotgunK2Vita-iteration13-draw-budget.vpk, con símbolos, snapshot, evidencia,
herramientas y hashes. Versiones anteriores intactas. Sin HUD propio.
FPS y estabilidad13 pendientes de consola.

Instalar sobre SGKV00001, sin borrar ux0:data/shotgunk2vita ni save.
Repetir toque de tablero/pausa/transición al perder o terminar un piso; probar
L1/R1, regreso a controles físicos, logro/Códice, sonido y Exit. Registrar menú
y partida60–120s con mismos relojes. Ideal comparar primer tablero cargado y
más adelante uno con más piezas/efectos. Enviar carpeta logs/00.13-... completa.

14 — Entity Profile: analizar entidades/callbacks/sprites junto a draw/update;
perfilar primitivas y sombras dominantes antes de cachear tiles o descartar
efectos. No quitar lógica para alcanzarFPS. Objetivo30 estables:presupuesto
33,3ms y P95 próximo a ese límite;60 necesita16,7ms, todavía pendiente.
