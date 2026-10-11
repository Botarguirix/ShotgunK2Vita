# Roadmap — Iteración 11 GPU Presenter

Confirmado en Vita 00.10: caché de entrada activa, 17,80 FPS en 1756 frames
válidos; logro detiene Lua por fb_ev ausente. DMA rechazada por integridad.

11 implementa callback de analítica local y presentación GXM: textura320×180,
escalado3×, L1/R1 táctiles, sincronización de cola y métricas sin HUD.
Regresiones PC y guardado del logro pasan; compilación Vita válida.
FPS, imagen GXM y estabilidad hardware todavía pendientes.

12 — Frame Budget: comparar recorridos CPU/GPU de la misma escena, perfilar
primitivas de dibujo y callbacks Lua que dominan draw/update; reducir trabajo
antes de añadir núcleos. Seguir media y P95, memoria/audio y sesión larga.
Aceptar 30 estables sólo con evidencia de frame cercano a33,3ms y sin errores;
60 exige16,7ms, por debajo de los29,23ms actuales de lógica+dibujo.
No comprimir atlases como supuesto arreglo de FPS: ya son indexados, sus
coordenadas deben conservarse y la compresión de archivo no reduce dibujo.

Instalación, límites y recorrido: [STATUS_ITERATION11.md](STATUS_ITERATION11.md).
Entregas 00.02–00.10 conservadas, nueva carpeta independiente 00.11.
