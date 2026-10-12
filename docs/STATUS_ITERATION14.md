# Iteración 14 — Controls and Shadows · 00.14

## Evidencia recibida00.13

Entrada00.13-1791780468-0 preservada en reports/iterations/00.14/input.
El usuario reporta mejora y un comportamiento intermitente de movimiento/animación,
con cruceta y stick izquierdo. Aclara que parece un fallo de lógica del juego:
el turno avanza y el sprite del rey queda atrás.
28727 muestras activas:26,23FPS mixtos,38,120ms/frame y P95 50,081ms.
Update7,213ms, draw13,675, present1,721, subida0,609, envíoGXM0,223,
espera13,145. CPU observado500MHz. Cierre por Exit, sin error Lua en esta sesión.
Entidades raíz media78,2/máximo279; callbacks de dibujo86,4/máximo326.
Correlación con draw_ms:entidades0,494, callbacks0,754, sprites0,719; no prueba
causalidad. No basta para garantizar60FPS en juego:presupuesto16,7ms.
El log registra82 cambios a Android. Se estudia ese cambio, sin atribuirle por
sí solo el fallo lógico/visual que describe el usuario.

## Controles solicitados

El adaptador pasa botones físicos directamente, sin combinaciones SELECT ni
consumo de botones al soltarlos. SELECT solo mantiene información y START pausa.
Se elimina también START+SELECT como salida de aplicación. Salida normal:Exit
en menú; en pantalla de error:START solo. No contador propio en pantalla.
El frontal sólo recoge L1/R1 en esquinas inferiores. El resto de toques se
ignora, no se envía como ratón ni se cambia a Android desde el frontal.
Se conserva L/R físico para acción arriesgada/disparo, y × para confirmar/mover.
L1/R1 táctiles cambian páginas. Diagrama actualizado, sin atajos L3/R3.
No se altera la API básica de toque de los ensayos internos; la política física
se aplica en ambos bucles de main (GPU y CPU).

## Movimiento y animación

Prueba de partida real:selección con cruceta y, por separado, stick izquierdo;
confirmar con×; comprobar cambio de hero.sq y final de tween en las coordenadas
de esa misma casilla. Ambos controles pasan en dos partidas consecutivas.
No se ha reproducido de manera concluyente el sprite fijo informado en Vita.
El primer supuesto de que un solo toque dejaba Android permanentemente activo
no se confirmó:el juego vuelve al mando. Los ensayos iniciales fallidos del
harness se conservan como tales, no como prueba del fallo original.
No se fuerzan posiciones ni se salta validación de movimientos del juego.

La segunda partida del ensayo detectó otro defecto:require devolvía un valor
cacheado de un entorno anterior en lugar de ejecutar el modo en el entorno
nuevo de safe_require; mode.start quedaba nil. La caché sigue en el entorno
raíz, pero los entornos temporales se cargan de nuevo. Prueba antes/después:
primer juego correcto/segundo start nil → dos juegos y ambos movimientos PASS.
Es una corrección separada; no demuestra la causa del sprite detenido físico.


Se añaden eventos move_request, hero_goto y game_rejected_action a crash-trace;
control_state en runtime registra modo, mando/toque, casilla del rey, posición
del sprite, coordenada de casilla, tween y objetivo seleccionado cada250ms.
Añade edad del rey, progreso/duración y destino de tween para detectar una
animación que deja de avanzar pese a que el turno continúa.
Conserva argumentos y resultados de las funciones originales. Esto distingue
solicitud rechazada de casilla cambiada con animación incompleta.
Si vuelve a ocurrir, recuperar la sesión completa y precisar momento/turno.

## Sombras y objetivo60FPS

shpr procesaba cada píxel mediante varias llamadas Lua/C:sget, pget, bright,
pset. La versiónC mantiene orden de columnas/filas, máscara, coordenadas,
cámara, recorte, paleta, patrón y mezcla. Preserva alias de superficies.
Si bright o las primitivas han sido sustituidas, usa la función Lua original.
Registra shadow_calls, shadow_pixels (intentos con máscara activa) y
shadow_fallbacks. No elimina casillas, piezas, sombras ni reglas del juego.
Límites:región hasta4096×4096 y coordenadas finitas; textos/shaders intactos.

Equivalencia por hash en6 casos:normal, cámara/clip, patrón/mezcla,
coordenadas fraccionarias negativas, superficie compartida y bright sustituido.
Ensayo final PC de3000 sombras:665673→17707µs (~37,6× para esa operación concreta).
No es ganancia de FPS ni medida ARM; efecto en partida pendiente en Vita.
Las métricas identificarán el tiempo restante en callbacks/Lua antes de seguir
optimizando hacia16,7ms/frame. El log13 de escena mixta no predice tasa de14.

## Validación y entrega

Regresión final ASan/UBSan:PASS (checks/run-FcnY8c). Consultar checks-delivery-run.log/result.txt,
controller-movement.log y shadow-budget.log en evidence para la prueba completa.
Cubre controles sin atajos, zonas frontales, casilla/sprite, combate6000frames,
pausa, logros/Códice, opciones, texto, pad, presentación y diagnósticos.
VitaSDK Release enlazado. Se añadió un margen virtual explícito antes de datos
para los metadatos SCE:el primer ELF no tenía espacio suficiente; recompilación
SELF/VPK correcta, sin depender de parchear el SDK. Segmentos guardados como
elf-segments-final.txt. Validación estática PASS: SFO00.14, ZIP/CRC y LiveArea.

Paquete independiente artifacts/iterations/00.14/
ShotgunK2Vita-iteration14-controls-and-shadows.vpk. Símbolos, fuente,
evidencia, herramientas y hashes; versiones anteriores intactas.
Instalar sobre SGKV00001 sin borrar ux0:data/shotgunk2vita ni save.
Probar ambos métodos de selección,×, apuntado/disparo, acciones especiales,
L1/R1 táctiles y toque central sin efecto. Salir mediante Exit; ante error,
START solo. Recorrer la misma partida60–120s con mismos relojes y recuperar
logs/00.14-...; si reaparece el desajuste, indicar turno y si avanza el enemigo.

15 — Lua Budget:perfilar update/draw restantes con datos14; investigar el fallo
lógico/animación con trazas reales. Mantener equivalencia y estabilidad mientras
se persigue60FPS; no anunciar60 estables antes de una medición física.
