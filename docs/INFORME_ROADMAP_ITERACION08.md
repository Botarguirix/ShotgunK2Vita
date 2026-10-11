# Informe de roadmap · iteración 00.08

## Progreso confirmado

El usuario prueba 00.07 y comunica fallo al disparar con R, FPS bajos y
parpadeo general. Los logs identifican `aspr()` ausente y otro fallo de opciones
por `bget()`. 00.08 corrige esas rutas y pasa las regresiones en PC.
[Evidencia y validación](STATUS_ITERATION08.md).

## Trabajo actual

Doble framebuffer, composición en RAM y dibujo de texto optimizado. El juego
ya funciona con escena lógica 320×180 e imágenes indexadas. La carga serial
de Lua/dibujo sigue fuera de la paralelización de presentación. La versión
no se declara estable hasta verificar los cambios en hardware y más rutas.

## Requisitos para una versión estable

| Área | Criterio de aceptación | Estado |
|---|---|---|
| Controles | Navegación, ambos sticks, disparo/recarga, acciones especiales y opciones | Regresiones PC; 00.08 pendiente en Vita |
| Compatibilidad | Sin APIs ausentes en rutas de modos/cartas admitidos | Auditoría identifica pendientes; no cobertura completa |
| Presentación | Sin escritura sobre scanout ni parpadeo persistente | Doble búfer implementado; comprobar en Vita |
| Rendimiento | Tiempos medidos por pantalla, sostenidos sin grandes picos | Referencia 00.07 ~19,53 FPS; 00.08 pendiente |
| Audio | Música/efectos audibles, sin interrupciones | Backend activo; bloques tardíos en 00.07; verificar |
| Persistencia | Guardar/cerrar/reabrir y recuperar partida/opciones sin pérdida | Saves codificados y bancos incompletos; pendiente |
| Partidas | Varios pisos, cartas, victoria/derrota y vuelta al menú | 6000 frames PC; campaña completa pendiente |
| Diagnóstico | Cada fallo deja sesión y compilación identificables | Logs por sesión, ELF/map y auditoría de entrega |

## Pendientes identificados

`tools/stability_audit.py` inventaría dependencias SUGAR conocidas y recursos
sin modificarlos. El análisis es por tokens; no garantiza cobertura de ramas
ni comprueba la semántica completa de funciones registradas. Bancos/saves
antiguos y mods usan APIs aún ausentes; también hay operaciones gráficas y
servicios auxiliares pendientes. Algunas rutas están condicionadas a modos
no admitidos o desactivados, por lo que cada hallazgo necesita clasificación.

Prioridad: confirmar disparo, opciones y doble búfer en OLED; medir U/D/P/copia;
asegurar persistencia de partida; ampliar recorridos de cartas y pisos.
Si sigue dominando presentación, evaluar composición/escalado por GPU. Si
domina Lua, perfilar callbacks y asignaciones antes de cambiar de intérprete.
Los tres participantes de presentación no son tres hilos de lógica Lua.

## Siguiente prueba

Mismo recorrido de título, tablero y disparos, balanced frente a single.
Enviar cada carpeta de sesión completa; comentar si desapareció el parpadeo
y si música/efectos se oyen. Un error nuevo requiere su traza y el VPK exacto.
