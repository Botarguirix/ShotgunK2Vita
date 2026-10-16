# Iteración16 — Half Resolution and Thread Audit (00.16)

Instalar artifacts/iterations/00.16/ShotgunK2Vita-iteration16-half-resolution-thread-audit.vpk
con VitaShell sobre SGKV00001. Conservar ux0:data/shotgunk2vita y save.
No desinstalar ni copiar nuevamente datos/APK/data.sgr. Actualiza el ejecutable.
Las entregas anteriores permanecen en sus carpetas: [ITERATIONS](ITERATIONS.md).
SFO/ZIP/LiveArea se verifican; prueba física15 pendiente.

## Controles actuales

Cruceta/stick izquierdo seleccionan destino;× confirma movimiento. Stick
derecho apunta, R dispara, L habilita acción arriesgada.□ especial, triángulo
recarga,○ cancela. START pausa, SELECT información. No atajos con SELECT.
Sólo esquinas frontales L1/R1 cambian páginas. Todo toque central se ignora.
L3/R3 no se emulan. Salida normal:menú→Exit. Si el juego muestra error:START solo.
[Diagrama de controles14, también válido para15](images/controls-vita.png).
HUD propio retirado.

## Verificación

1. Abrir título/menú y empezar partida; mantener mismos relojes del plugin.
2. Probar movimiento con cruceta y stick por separado, confirmar con× y
   comprobar casilla, sprite y respuesta de enemigos. Probar apuntado/disparo.
3. Tocar centro del tablero:no debe actuar. L1/R1 deben seguir funcionando.
4. Perder y pulsar×/R durante Game Over; reintentar y jugar otra partida.
5. Verificar pausa, logro y Códice; volver al menú y salir con Exit.
6. Recuperar carpeta completa logs/00.15-... después de60–120s; si el movimiento
   falla, indicar turno/acción y si avanza el turno pese a la imagen detenida.

runtime.log incluye control_state (casilla, sprite, edad, progreso de tween y destino).
crash-trace.log registra move_request, hero_goto y game_rejected_action.
performance.csv añade métricas de sombras. No se ha demostrado todavía la
causa completa del desajuste informado ni60FPS estables; véase [STATUS](STATUS_ITERATION15.md).
v15 corrige controles que permanecían activos tras retirarlos y evita disparar
con rey muerto/sin casilla. El parche se aplica al cargar Lua: no copiar datos nuevos.
