ShotgunK2Vita Performance Tool

1. Copiar performance.csv de ux0:data/shotgunk2vita/ al PC tras salir del juego.
2. Abrir PerformanceTool.cmd y seleccionar ese CSV. Requiere Python 3.
3. El informe HTML se abre en el navegador y funciona sin conexión.

Para ajustar rendimiento, copiar uno de presets/balanced.cfg, single.cfg o
battery.cfg a ux0:data/shotgunk2vita/performance.cfg. Reiniciar el juego.
Comparar los mismos 60 segundos de título, menú y tablero. Conservar un CSV
por corrida; el juego lo reemplaza en cada arranque. No reemplazar save/.

balanced: CPU 444 MHz, presentación 3 participantes.
single: CPU 444 MHz, presentación 1 participante (comparación A/B).
battery: CPU 333 MHz, presentación 2 participantes.

SELECT+TRIÁNGULO muestra/oculta el HUD.
Audio tiene su propio audio.log; el CSV no mide la carga de su hilo.
