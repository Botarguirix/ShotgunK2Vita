# Herramienta de rendimiento · versión 00.03

El VPK escribe `ux0:data/shotgunk2vita/performance.csv`. El HUD muestra FPS y
milisegundos de lógica (U), dibujo (D) y presentación (P). SELECT+TRIÁNGULO
lo oculta o muestra. `W` indica cuántos participantes tiene la presentación;
no es un porcentaje de utilización de CPU.

## Medir en Vita

1. Instalar el VPK 00.03 conservando la carpeta de datos y `save/`.
2. Hacer un recorrido de unos 60 segundos: título, menú y primer tablero.
3. Salir con START+SELECT y copiar `performance.csv`, `runtime.log` y
   `audio.log` antes de volver a abrir el juego: los registros se reemplazan.
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
333 y 444; `workers` acepta 1–3. `hud=0` oculta el HUD y `profile=0` desactiva
el CSV para una prueba sin registro. El reloj previo se restaura al salir por
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
