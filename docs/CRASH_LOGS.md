# Recuperar diagnóstico desde la iteración 00.05

Instalar el VPK actual de `artifacts/iterations/00.08/` sobre el mismo título.
Conservar `ux0:data/shotgunk2vita/` y todas las partidas. No requiere plugins
de diagnóstico ni copiar de nuevo los datos del juego.

## Después de un crasheo

1. Abrir VitaShell y entrar en `ux0:data/shotgunk2vita/logs/`.
2. Copiar **toda la carpeta `logs`** al PC mediante USB o FTP. Cada ejecución
   tiene su propia carpeta `<versión>-<identificador>-<n>/`; `latest.txt` indica la
   más reciente. Si abriste otra vez el juego, conservar también la anterior.
3. Dentro de cada sesión están `runtime.log`, `audio.log`, `performance.csv`
   (si `profile=1`), `crash-trace.log` y `console.log` para stdout/stderr.
   Comprimir la carpeta y adjuntar el ZIP.
4. Si el aviso de Vita informa que guardó un core, buscar el `.psp2dmp` reciente
   en `ux0:data/` y copiarlo también. No todas las salidas generan un volcado.
5. Indicar código de error, pantalla/acción anterior, FPS aproximados y si se
   escuchan música y efectos. No borrar los logs hasta terminar el diagnóstico.

## Qué conserva

Los logs se abren antes de cargar el juego o reservar el framebuffer.
Un hilo independiente guarda checkpoints aproximadamente cada 250 ms y solicita
sincronizar `ux0:`. Registra fase de update/draw/presentación/espera, frame,
botones, sticks, última ubicación Lua observada por el hook, memoria Lua y
superficies, estado de workers y progreso/resultado del audio. Los eventos
anotan cargas de scripts/imágenes y errores. El código Lua con error conserva
la traza de llamadas en `runtime.log`; los errores Lua recuperables muestran
una pantalla de parada para poder salir con START+SELECT.

`session_end=clean` indica salida con START+SELECT. `session_end=lua_error`
indica un error Lua registrado seguido de salida. La ausencia de `session_end`
puede indicar crash, cierre forzado o copia incompleta; no demuestra por sí sola
un fallo de memoria. Un fallo nativo no siempre permite ejecutar más código:
el checkpoint es una muestra anterior, no una traza exacta de la excepción.
La sincronización puede retrasarse por carga o errores del almacenamiento.

`lua_kb` no mide todo el heap y `graphics_kb` cuenta superficies, sin fuentes
ni audio. La ubicación Lua es la última muestra del hook de 10.000 instrucciones.
En `workers`, 1 significa rasterizando y 0 significa fuera del rasterizado.
`dropped_events` detecta eventos omitidos al llenarse la cola o por contención.
El trace limita su crecimiento a 16 MiB por sesión; no borra sesiones anteriores.
Estas escrituras añaden carga; comparar el rendimiento medido con esta versión.

## Preparar un ZIP en PC (opcional)

```sh
python3 tools/collect_diagnostics.py /ruta/logs diagnostico-00.06.zip
# Con un volcado nativo:
python3 tools/collect_diagnostics.py /ruta/logs diagnostico-con-core.zip --dump /ruta/fallo.psp2dmp
```

La herramienta incluye logs, CSV, resumen de checkpoints y los dumps indicados.
No incluye recursos del juego ni saves, no sube archivos y rechaza reemplazar un
ZIP existente. También funciona enviando un ZIP hecho desde el explorador.

## Símbolos para el análisis

Cada entrega desde 00.05 incluye `debug/ShotgunK2Vita.elf` y `.map` correspondientes
al VPK. Se conserva optimización Release y se añade información `-g`.
Estos archivos quedan en PC: no instalarlos en Vita. Sirven para resolver
direcciones del volcado hasta la función/línea del código de esa iteración.

Referencias: [sincronización de archivos VitaSDK](https://docs.vitasdk.org/group__SceFcntlUser.html),
[ubicación y análisis de cores Vita](https://github.com/xyzz/vita-parse-core).
