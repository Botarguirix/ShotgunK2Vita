# Roadmap — Iteración14 Controls and Shadows

Vita13:mejora informada,26,23FPS mixtos; callbacks/sprites acompañan coste de
dibujo. Movimiento intermitente reportado con cruceta y stick; sin traceback.

14:sin atajos SELECT; frontal sóloL1/R1; START pausa y Exit sale. Sombras enC
con fallback equivalente y métricas. Diagnóstico separa solicitud, casilla y
animación. La causa del desajuste físico aún no está reproducida/concluida.
Se corrigió además la carga en un nuevo entorno al reiniciar otra partida:
dos partidas consecutivas y movimiento con ambos controles pasan en PC.
Regresiones PC y prueba real de movimiento descritas en STATUS_ITERATION14.

15 — Lua Budget:medir coste restante tras sombras, perfilar callbacks/update y
analizar trazas del movimiento. Objetivo60 requiere16,7ms/frame y estabilidad
física, no un resultado de microbenchmark. Preservar reglas, sonido y gráficos.

[Estado, pruebas y entrega](STATUS_ITERATION14.md). Entregas anteriores intactas.
