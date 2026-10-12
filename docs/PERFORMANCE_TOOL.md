# Herramienta de rendimiento · versión 00.13

Desde 00.05, el CSV se guarda en cada sesión:
`ux0:data/shotgunk2vita/logs/<versión>-<identificador>-<n>/performance.csv`.
Usar ese archivo para los informes; [recuperación de logs](CRASH_LOGS.md).
Nuestro contador de pantalla está retirado. `hud` antiguo se ignora; el plugin
puede mostrar FPS. Las métricas internas se compilan y siguen guardándose en
CSV/logs. Los participantes de presentación no indican utilización de CPU.

## Medir en Vita

1. Instalar el VPK 00.13 conservando la carpeta de datos y `save/`.
2. Hacer un recorrido de unos 60 segundos: título, menú y primer tablero.
3. Salir con START+SELECT y copiar la carpeta de esa sesión dentro de `logs/`.
   Incluye `performance.csv`, `runtime.log`, `audio.log` y `crash-trace.log`;
   las ejecuciones anteriores se conservan al abrir de nuevo.
4. En Windows, abrir `PerformanceTool.cmd` y seleccionar el CSV. Requiere
   Python 3; encuentra primero el Python instalado por Codex y luego `python`.
   El informe HTML funciona sin conexión y permite ocultar etapas del gráfico.

También funciona por consola:

```sh
python tools/perf_tool.py report performance.csv --output informe.html
python tools/perf_tool.py report nueva.csv --baseline anterior.csv --output comparacion.html
```

La media refleja el tiempo entre presentaciones, incluyendo el coste de
registro de la vuelta anterior. P95 muestra los fotogramas más lentos.
Los primeros fotogramas pueden contener cargas y letras aún sin cachear;
`--skip-first 120` permite excluirlos si se desea medir sólo el estado estable.
No comparar pantallas o recorridos diferentes como si fueran equivalentes.

## Ajustar y comparar

```sh
python tools/perf_tool.py config --preset balanced --output performance.cfg
```

Copiar el archivo a `ux0:data/shotgunk2vita/performance.cfg` y reiniciar el juego.

| Preset | CPU solicitada | Participantes de presentación | Uso |
|---|---:|---:|---|
| balanced | 444 MHz | 3 | Punto de partida |
| single | 444 MHz | 1 | Comparar el beneficio del reparto entre núcleos |
| battery | 333 MHz | 2 | Comparar consumo/rendimiento |

Sin archivo se usa balanced. Los únicos valores aceptados de `cpu_mhz` son
333 y 444; `workers` acepta 1–3. `profile=0` desactiva
el CSV; los logs de diagnóstico siguen activos en 00.05. El reloj previo se restaura al salir por
START+SELECT. El log informa si Vita rechazó una frecuencia o un hilo.

La aplicación usa los tres núcleos disponibles para aplicaciones:
Lua/dibujo y parte de la presentación en CPU 0; filas en CPU 1 y CPU 2.
El hilo de audio tiene mayor prioridad y usa CPU 2. Los workers esperan
semáforos cuando no tienen trabajo; no se busca saturar todos los núcleos.
No se ejecuta el mismo estado Lua simultáneamente desde varios hilos.
Referencia: [máscaras de CPU de VitaSDK](https://docs.vitasdk.org/group__SceCpuUser.html).

## Qué corregir según el informe

| Etapa dominante | Siguiente investigación |
|---|---|
| U / update | Perfil de funciones Lua; reducir asignaciones y coste de entrada; evaluar LuaJIT si sigue dominando |
| D / draw | Llamadas gráficas, tamaño de regiones, fuentes; estudiar GPU/GXM si el software sigue siendo insuficiente |
| P / present | Comparar single/balanced; revisar transferencia y composición |
| wait | Comprobar VSync antes de atribuir ese tiempo a trabajo de CPU |

`runtime.log` incluye núcleos observados y tiempo de filas por participante.
`audio.log` incluye apertura del puerto, pistas y bloques cuya preparación
superó los 46,44 ms disponibles. El CSV no mide la carga de CPU del audio.

Las pruebas en PC se marcan como tales: sus FPS derivados son una medida
del tiempo de trabajo sin VSync; no son FPS medidos ni predichos para Vita.

## Cambios de 00.08

`copy_ms` mide la copia RAM a pantalla; `running` identifica frames activos.
El informe excluye frames detenidos. Para CSV antiguos, mantener `runtime.log`
en la misma carpeta: el primer checkpoint running=0 es un límite aproximado.
La copia no está incluida en present_ms; consultar ambos para evaluar el coste
total. El nuevo ZIP se genera dentro de cada entrega con el código vigente.

[Auditoría de APIs e imágenes](STABILITY_AUDIT.md).

## Frame Transfer: comparar CPU y DMA

`balanced`, `single` y `battery` usan `copy_mode=2`: ocho patrones distintos
verifican la copia DMA y miden ambas rutas; se elige la de menor tiempo total.
`copy-cpu` fuerza CPU y `copy-dma` solicita DMA, manteniendo fallback por error.
El ensayo usa el framebuffer inactivo antes de mostrar la pantalla inicial.
No se publica un búfer hasta terminar la copia síncrona. El CSV añade
`copy_mode` (0 CPU, 1 DMA) y `copy_rc`; `runtime.log` registra `copy_probe`.
La prueba inicial no sustituye una sesión de hardware: comparar `copy_ms`,
media/P95 de frame y parpadeo durante menú/tablero con idéntico recorrido.

## Input Cache: métricas 00.10

CSV e informe añaden input_calls (consultas), input_evaluations (valores de
un binding calculados por frame) e input_parses (compilaciones de binding).
La caché se invalida al reasignar y en cada frame; acepta botones/sticks/toque,
flancos y liberación. No confundir estos contadores con porcentaje de CPU.
Comparar update_ms y frame_ms en recorridos iguales con reloj y workers iguales.
El ensayo sintético PC de 1000 frames/602000 consultas pasó en 00.09 y 00.10:
2549971 frente a 370225 µs de update total. No predice FPS de consola.

Los rótulos L1/R1 son controles táctiles, no un HUD de rendimiento.

## GPU y comparación 00.11

balanced/battery seleccionan renderer=1, escalado GPU. copy-cpu y single
seleccionan renderer=0 para comparar CPU anterior. workers sólo controla
la presentación CPU; GPU compone en un participante y escala con GXM.
No forzar copy-dma: en 00.10 el autotest falló integridad aun con rc=0.
El CSV identifica renderer y gpu_submit_ms; copy_ms mide subida de textura,
wait_ms incluye cola GPU/vblank. No interpretar espera como trabajo de Lua.
Repetir misma escena/relojes, comparar media/P95 y excluir running=0.

00.12 mantiene el esquema GPU/CPU y optimiza present_ms. Comparar misma partida,
relojes y ventana; el benchmark PC no predice el incremento de FPS en Vita.

## Entidades00.13

entities es tamaño de lista raíz; entity_updates/entity_draws son llamadas al
callback (hijos y listas extra pueden superar entities; invisibles también
pueden recibir la llamada). palette_resets cuenta resets y palette_suppressed
la ruta de pal Lua sustituido durante pausa/otros. sprite_calls cuenta llamadas
blit, no tiles; blit_pixels mide área procesada. No reducir entidades sin medir.
El informe da media/máximo y correlación de estos contadores con draw_ms.
Correlación no demuestra causalidad; mezclar menú/partida puede ocultar el coste.
Comparar misma escena/relojes, media/P95 y excluir running=0.
