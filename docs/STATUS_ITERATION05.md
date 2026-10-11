# Iteración 00.05 · diagnóstico persistente

El usuario informa mejora considerable de FPS tras la entrega anterior,
pero un crasheo sin log recuperado. Todavía no hay mediciones exactas ni causa
del crasheo identificada. No se afirma que esta iteración solucione ese fallo.

La nueva entrega crea una carpeta por ejecución desde el inicio de main.
Runtime, audio, CSV y consola se guardan allí. Un hilo independiente conserva
checkpoints ~250 ms con fase, entradas, ubicación Lua, memoria parcial, workers
y progreso del audio, y solicita sincronización del dispositivo. Los errores
Lua recuperables guardan la traza de llamadas. Se corrigió además el límite
vertical del texto de diagnóstico al envolver una línea larga.

Validación realizada:

- Compilación nativa Vita en Release, con símbolos `-g` y mapa de enlace.
  Confirmada la sección `.debug_info` en el ELF guardado junto al VPK.
- Fixture de error Lua: mensaje, funciones inner/outer y líneas en el log.
- Fixture de terminación abrupta sin cierre de archivos: checkpoint conservado;
  la siguiente ejecución crea otra carpeta y conserva intacta la anterior.
- AddressSanitizer y UndefinedBehaviorSanitizer: mismo fixture sin diagnósticos.
- Recorrido de 4.800 frames hasta partida activa, también con el hilo de
  diagnóstico activado y tres participantes de presentación en PC.
- Prueba de controles y mezcla de cinco minutos de audio MP3/WAV en PC.
- Empaquetador de ZIP de diagnóstico probado con varias sesiones locales.
- VPK: SFO 00.05/SGKV00001, integridad ZIP, dimensiones PNG, índice de 8 bits,
  paleta opaca de 256 entradas y referencias XML validados.

Las pruebas de PC no ejercen las APIs de almacenamiento/audio/hilos de Vita.
La próxima prueba debe recuperar los logs reales y, si se generó, el core
`.psp2dmp`; permitirá decidir el arreglo a partir de evidencia.

Entrega: `artifacts/iterations/00.05/`. Las versiones anteriores se conservan.
[Recuperar logs](CRASH_LOGS.md) · [Roadmap](ROADMAP.md).
