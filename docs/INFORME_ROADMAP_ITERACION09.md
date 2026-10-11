# Informe de avance · Iteración 09 — Frame Transfer

00.08 mejora mucho según el usuario y ahora la evidencia identifica otra ruta
faltante: ismobile al pausar, seguido por el índice OPTIONS[id]. 09 registra el contrato, retira el contador propio
manteniendo métricas y añade comparación de transferencia CPU/DMA.
La referencia recibida equivale a 18 FPS en 7036 frames activos; no es un
resultado de 09. Dibujar cuesta 19,86 ms y transferir 8,41 ms en esa sesión.

Entrega actual: 00.09 Frame Transfer. Siguiente evidencia: pausa/opciones,
recorrido comparable con perfiles copy-cpu/copy-dma, media/P95, imagen y audio.
Etapas propuestas: 00.10 Input Cache, 00.11 GPU Presenter, 00.12 Frame Budget.
Cada versión futura tendrá su propia carpeta, símbolos, snapshot y métricas;
no se generan VPKs de etapas que aún no están implementadas y comprobadas.

[Referencias y aplicación](PORT_OPTIMIZATION_RESEARCH.md),
[estado y aceptación](STATUS_ITERATION09.md), [índice](ITERATIONS.md).
