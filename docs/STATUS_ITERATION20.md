# 00.20 — Analog Deadzone and Scenario Sweep

Se eleva la zona muerta analógica de 24 a 32 unidades alrededor de 128. El
objetivo es evitar que la deriva del stick mueva el cursor en Codex/Logros y
mantener el evento `btnp` cuando el usuario cruza el umbral de forma real.
La habilidad `srand` de 00.19 se conserva.

Esta entrega debe probar menú, Codex, Logros, tablero, apuntado, habilidad,
Game Over, reinicio y salida. El rendimiento de 60 FPS sigue pendiente de
medición física; la zona muerta solo reduce entradas espurias.
