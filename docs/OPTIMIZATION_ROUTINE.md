# Rutina de optimización por pases

Objetivo: reducir el coste por frame conservando reglas, imagen, sonido y
estabilidad. El presupuesto físico es 16,67 ms para 60 FPS y 33,33 ms para
30 FPS. Un PC rápido no demuestra que se cumpla en la PS Vita.

## Ejecutar una campaña local

En WSL, desde el proyecto:

```bash
bash scripts/optimize_passes.sh
```

Primero ejecuta la regresión ASan/UBSan. Si falla, detiene la campaña. Después
compila exclusivamente el banco PC en Release: las mediciones no incluyen
sanitizadores. Usa tres escenas: menú, tablero iniciado sin acciones y tablero
con una secuencia fija de apuntado, disparo, recarga y movimiento. La tercera
escena puede evolucionar o terminar: compara la misma secuencia, no se presenta
como una prueba permanente de carga máxima ni como una partida completa.

Hace tres repeticiones de 1200 frames con uno y tres participantes de
presentación CPU: 18 ejecuciones en total. Descarta 120 frames de calentamiento,
rota el orden de los participantes y fija la semilla. Cada caso usa una copia
independiente de los recursos/partida preparada. Al finalizar un caso elimina
sólo su fixture; mantiene CSV, log y métricas. Si falla, conserva el fixture.
Para evitar copiar repetidamente por el montaje Windows/WSL, prepara una caché
temporal local y clona allí cada caso; ante fallo copia el fixture a la evidencia.

Las imágenes de todos los frames generan una huella fuera del intervalo medido.
Si cambia entre repeticiones o número de participantes, se rechaza la campaña.
Esta comprobación detecta diferencias de dibujo; las pruebas de reglas y
movimiento siguen siendo necesarias. El banco PC usa presentación CPU, mientras
que el port físico puede usar GPU: no permite seleccionar directamente el mejor
renderer o número de CPUs de la Vita.

Resultados únicos: `reports/campaigns/00.NN/run-.../report.html` y
`summary.json`. Conserva media/P95 por pase, medianas, variación entre pases,
tiempo por etapa, huellas, versión, hash del ejecutable y estado Git.

## Comparar una optimización

Guardar la primera campaña como referencia. Cambiar un único coste identificado
por las etapas; no cambiar simultáneamente resolución, reglas y sincronización.
Repetir la campaña con el mismo PC, recursos, semilla y carga externa baja:

```bash
bash scripts/optimize_passes.sh --baseline reports/campaigns/00.14/run-REFERENCIA/summary.json
```

Una diferencia inferior al 5%, o menor que la variación entre pases, queda como
inconclusa. Una reducción mayor se etiqueta **candidata**, no como certeza
estadística. Una huella distinta entre versiones requiere investigar y validar
visualmente; no acepta silenciosamente una mejora conseguida omitiendo dibujo.
Revisar también P95: una media menor con tirones peores necesita otra revisión.

## Vita3K y validación física

Usar `tools/test_vita3k.ps1` con el VPK candidato. Confirmar primero entrada:
actualmente el arranque SDL de v14 está probado, la navegación automatizada no.
Hasta resolverlo, no automatizar campañas de jugabilidad con supuestos inputs.
Vita3K prueba el binario ARM y las APIs; su VSync, caché de shaders y host GPU
pueden ocultar un coste. Separar el primer pase de compilación de shaders de
los tres pases medidos. Repetir la misma partida/escena y duración con idéntica
configuración, guardando `performance.csv`, runtime y audio por pase.

Analizar cada CSV con `tools/perf_tool.py report ... --baseline ...`; etiquetar
explícitamente el origen Vita3K. No mezclar esos CSV con campañas PC ni consola.
No convertir ratios PC a FPS simulados de Vita: el resultado sería especulativo.

Cuando haya un candidato sin regresiones: compilar y empaquetar una iteración
nueva siguiendo ITERATION_PROCESS.md. Probar en consola menú, tablero cargado,
acciones, audio, pausa/logros, segunda partida y salida, con los mismos relojes.
La aceptación requiere métricas físicas, imagen correcta y sprite sincronizado.
Sólo entonces retirar instrumentación temporal que ya no aporte diagnóstico;
conservar métricas útiles, evidencia y todas las entregas históricas.

## Primera referencia ejecutada, 2026-10-13

Regresión ASan/UBSan PASS: `checks/run-Xmamdy`. Campaña Release PASS:
`reports/campaigns/00.14/run-uishu64a`, 18 ejecuciones y huellas equivalentes
en cada escena. Pruebas del comparador: 5 PASS, incluyendo ruido, imagen distinta,
host distinto y regresión de P95. El intento `run-3kur8bwj` está excluido: su
preparación mostraba portada en vez del menú, corregido antes de la referencia.

| Escena | Participantes CPU | Mediana de medias PC (ms) | Mediana P95 PC (ms) |
|---|---:|---:|---:|
| Menú | 1 | 1,202 | 1,841 |
| Menú | 3 | 1,132 | 1,666 |
| Tablero quieto | 1 | 2,052 | 3,433 |
| Tablero quieto | 3 | 1,791 | 2,782 |
| Acciones | 1 | 1,883 | 2,889 |
| Acciones | 3 | 1,796 | 2,665 |

Es referencia PC, no comparación antes/después de una optimización. La variación
del tablero con un participante fue amplia (1,646–2,457 ms de media); se conserva
para no confundir ruido con ganancia. En este banco el menú concentra coste en
draw y el tablero/acciones en update. No representa tableros avanzados de máxima
carga ni demuestra 60 FPS en consola. Próximo paso: candidato acotado a la etapa
costosa, mismos pases y validación final de su VPK en hardware.
