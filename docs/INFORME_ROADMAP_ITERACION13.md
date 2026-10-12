# Roadmap — Iteración 13 Draw Budget

Vita12: gráficos/sonidos aprobados por usuario, menú cerca de30FPS; sesión
mixta22,18FPS, P95 66,74ms. Presentación1,75ms; update+draw30,23ms.
Fallo hero nil al tocar pantalla y dibujar una casilla sin rey presente.

13: guardia táctil específica, reset de paleta nativo equivalente y contadores
de entidades/callbacks/sprites. Regresiones sanitizadas y compilación Vita pasan.
Reset de referencia PC aproximadamente22,8× más lento que C para esa operación;
no extrapolar a FPS. Datos/saves, tablero y efectos conservados.

14 — Entity Profile: medir correlaciones por escena, perfilar lógica/dibujo,
identificar sombra/primitiva/callback dominante y optimizar con imagen y reglas
iguales. No eliminar entidades a ciegas. Confirmar30 estables con P95 y sesión
larga antes de perseguir60. Mantener audio y controles aprobados en12.

[Estado, evidencia y recorrido físico](STATUS_ITERATION13.md).
13 pendiente de medir en Vita; versiones previas conservadas.
