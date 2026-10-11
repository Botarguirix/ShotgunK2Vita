# Iteración 10 — Input Cache · 00.10

## Entrada y diagnóstico

Sesión original 00.09-1791740744-0 y captura recibida conservadas en input.
El usuario informa que no encuentra otros fallos de juego; Exit llama quit()
no registrado (code/menu.lua:1096). Referencia: 10168 frames activos, 18,38 FPS,
54,42 ms de media y P95 83,41. Lógica 13,29 ms, dibujo 20,18, presentación 6,09,
copia 5,54, espera 7,10. CPU observado 500 MHz y tres participantes.
DMA devuelve rc=0, pero falla integridad en pase 6: fallback CPU confirmado.
Se conserva la comprobación; DMA no se fuerza como una optimización garantizada.

## Cambios

Bindings compilados a máscaras de botones, direcciones de ejes y toque al
registrarlos; consultas reutilizan valores actual/flanco/anterior por frame.
Reasignar invalida inmediatamente. Tokens directos tienen caché acotada; textos
largos usan el mismo parser con truncado previo de 512 bytes. Semántica de
zona muerta y máximo analógico conservada. Se reinician cachés al arrancar.
No se ejecuta Lua simultáneamente en varios núcleos. Presentación sigue en
hasta tres participantes; el objetivo aquí es evitar trabajo repetido en CPU.
Quit marca una solicitud; main sale del bucle y conserva teardown de workers,
audio, CSV/logs, pantalla y memoria. Exit registra menu_exit, sin error Lua.
CSV/herramienta cuentan consultas, evaluaciones y parseos; HUD sigue retirado.

## Validación

Prueba idéntica contra librerías 00.09 y 00.10, ASan/UBSan: 1000 frames,
602000 consultas de botones, toque y sticks, flancos/liberaciones y reasignación
mientras se consulta el mismo frame. Ambos pasan. Update total PC 2549971 →
370225 µs (~85,5% menos en ese ensayo); 4000 evaluaciones y 2001 parseos en 10.
No es mejora medida del juego completo ni de FPS en Vita. PASS de la regresión común ASan/UBSan: 6000 frames de combate,
ACTUAL_PAUSE_ROUTE=passed y MENU_EXIT_REGRESSION=passed. Comprueba además el juego real, pausa, Exit, dibujo, texto, pad y logs.
Vita Release compilado, SFO 00.10, CRC/LiveArea válidos.
Resultados originales y compilación/validación VPK se incluyen en evidence.
Transferencia DMA y cierre de APIs Vita todavía requieren consola física.
Saves antiguos, todos los modos/cartas y audio avanzado siguen pendientes.

## Entrega y aceptación

artifacts/iterations/00.10/ShotgunK2Vita-iteration10-input-cache.vpk.
Instalar sobre SGKV00001 conservando ux0:data/shotgunk2vita/ y save/.
Mismo recorrido con reloj/perfil idénticos: título, menú y tablero; comprobar
sticks, cruceta mantenida, R, triángulo, combos y pausa. Elegir Exit al volver
al menú: cierre sin pantalla de error, con exit_reason=menu_exit.
Recuperar logs/00.10-* para tiempos y consultas/evaluaciones/parseos. FPS de 10
no se declaran hasta recibir la sesión. 09 permanece intacta.

L1/R1 táctiles: botones visibles en las esquinas inferiores del panel frontal.
Mantener equivale a mantener L1/R1; se pueden tocar ambos a la vez. Sus zonas
consumen el toque antes de pasarlo al tablero. SELECT+L y SELECT+R siguen siendo
alternativas físicas; L/R solos corresponden a L2/R2. No se añade un contador FPS.
