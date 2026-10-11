# Informe de avance · Iteración 10 — Input Cache

00.09 es jugable sin otros errores reportados; falla Exit por quit ausente.
10 añade cierre ordenado y reduce el trabajo repetido de entrada con cachés.
La sesión de referencia de Vita promedia 18,38 FPS; dibujo 20,18 ms y lógica
13,29 ms. El ensayo idéntico de controles PC conserva resultados y baja el
coste de update sintético ~85,5%; no es un resultado FPS del juego en consola.

Actual: 00.10 Input Cache. Hardware pendiente: recorrido comparable, controles,
Exit y logs terminados. Próxima: 00.11 GPU Presenter para reducir presentación
/escalado con GPU; evaluar también las primitivas dominantes de dibujo.
00.12 Frame Budget: perfilar Lua/primitivas y validar media/P95/partidas largas.
Mantener carpetas independientes, métricas internas y el plugin FPS del usuario.

[Estado/aceptación](STATUS_ITERATION10.md), [índice](ITERATIONS.md),
[referencias de otros ports](PORT_OPTIMIZATION_RESEARCH.md).

L1/R1 táctiles: botones visibles en las esquinas inferiores del panel frontal.
Mantener equivale a mantener L1/R1; se pueden tocar ambos a la vez. Sus zonas
consumen el toque antes de pasarlo al tablero. SELECT+L y SELECT+R siguen siendo
alternativas físicas; L/R solos corresponden a L2/R2. No se añade un contador FPS.
