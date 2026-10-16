# 00.16 — Half Resolution and Thread Audit

Esta iteración prepara una candidata local para Vita3K y para la siguiente prueba física. La resolución lógica sigue siendo 320×180; `resolution_divisor=2` presenta a 160×90 y escala por punto a 960×540. Se puede volver a `1` en `ux0:data/shotgunk2vita/performance.cfg` si la legibilidad no es suficiente.

El modo GPU usa el número de workers configurado (por defecto 3) para la copia/presentación. Se mantienen fences y el drenaje de GXM; no se paraleliza Lua ni se modifica la afinidad del sistema. `performance.csv` registra tiempos muestreados por familia gráfica, dimensiones y contadores.

El diagnóstico registra IDs, afinidad efectiva, CPU de última ejecución, prioridad, estado, tipo de espera y deltas de ciclos de los hilos propios, además de la duración de `sceIoSync`. Los hilos internos de GXM/kernel quedan fuera; no se afirma que se usen todos los hilos de Vita.

La compilación VitaSDK terminó correctamente. La regresión ASan/UBSan, campañas PC y Vita3K deben quedar registradas antes de congelar el VPK; ninguna sustituye una medición en consola.

## Prueba física

Instalar el VPK conservando `ux0:data/shotgunk2vita` y partidas. Medir menú, tablero con muchas entidades, derrota/reinicio y salida. Recuperar `runtime.log`, `performance.csv`, `audio.log` y `crash-trace.log`; indicar si 160×90 es legible y anotar FPS del plugin. Si no lo es, cambiar el divisor a 1 y repetir la misma ruta.
