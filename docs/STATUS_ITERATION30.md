# Iteración 00.30 — Integrated Performance Candidate

Esta entrega integra los cambios de 00.26–00.29 en una compilación final para
comparación: overlay GPU limpio, índice O(1) de glifos bitmap, recorte de
overdraw de sombras y ruta rápida de silencio en audio. Mantiene salida lógica
320×180, controles traseros L1/R1 y servicios externos desactivados.

La selección se basa en regresiones locales, equivalencia gráfica/audio y
arranque Vita3K. La aceptación de FPS, audio audible y estabilidad sigue siendo
una prueba de consola OLED con PSVShell.
