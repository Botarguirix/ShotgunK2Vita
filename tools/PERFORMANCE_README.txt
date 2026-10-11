ShotgunK2Vita Performance Tool

1. Copiar toda la sesión de ux0:data/shotgunk2vita/logs/ al PC tras salir del juego.
2. Abrir PerformanceTool.cmd y seleccionar ese CSV. Requiere Python 3.
3. El informe HTML se abre en el navegador y funciona sin conexión.

Para ajustar rendimiento, copiar uno de presets/balanced.cfg, single.cfg o
battery.cfg a ux0:data/shotgunk2vita/performance.cfg. Reiniciar el juego.
Comparar los mismos 60 segundos de título, menú y tablero. Conservar un CSV
por corrida; cada ejecución tiene su propia carpeta. No reemplazar save/.

balanced: CPU 444 MHz, presentación 3 participantes.
single: CPU 444 MHz, presentación 1 participante (comparación A/B).
battery: CPU 333 MHz, presentación 2 participantes.

00.09 retira el contador propio; las métricas siguen en CSV y logs.
copy-cpu/copy-dma comparan transferencias; balanced selecciona al arrancar.
Audio tiene su propio audio.log; el CSV no mide la carga de su hilo.

00.08: excluye muestras detenidas (running=0 o runtime.log contiguo) e informa
copy_ms de RAM a pantalla por separado. Para versiones antiguas mantener CSV
y runtime.log juntos: el primer checkpoint running=0 es un límite aproximado.

Auditar datos preparados sin modificarlos:
python stability_audit.py RUTA_DATOS --output auditoria-nueva.json
Enumera dependencias SUGAR conocidas y tamaños de imágenes. No prueba todas las
rutas de juego. SGI1 ya usa un byte por píxel; comprimir reduce almacenamiento,
no el trabajo de dibujo de cada frame. No redimensionar atlas de sprites.
