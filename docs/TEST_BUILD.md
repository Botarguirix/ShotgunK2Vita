# Iteración14 — Controls and Shadows (00.14)

Instalar artifacts/iterations/00.14/ShotgunK2Vita-iteration14-controls-and-shadows.vpk
con VitaShell sobre SGKV00001. Conservar ux0:data/shotgunk2vita y save.
No desinstalar ni copiar nuevamente datos/APK/data.sgr. Actualiza el ejecutable.
Las entregas anteriores permanecen en sus carpetas: [ITERATIONS](ITERATIONS.md).
SFO/ZIP/LiveArea se verifican; prueba física14 pendiente.

## Controles actuales

Cruceta/stick izquierdo seleccionan destino;× confirma movimiento. Stick
derecho apunta, R dispara, L habilita acción arriesgada.□ especial, triángulo
recarga,○ cancela. START pausa, SELECT información. No atajos con SELECT.
Sólo esquinas frontales L1/R1 cambian páginas. Todo toque central se ignora.
L3/R3 no se emulan. Salida normal:menú→Exit. Si el juego muestra error:START solo.
[Diagrama actualizado](images/controls-vita.png). HUD propio retirado.

## Verificación

1. Abrir título/menú y empezar partida; mantener mismos relojes del plugin.
2. Probar movimiento con cruceta y stick por separado, confirmar con× y
   comprobar casilla, sprite y respuesta de enemigos. Probar apuntado/disparo.
3. Tocar centro del tablero:no debe actuar. L1/R1 deben seguir funcionando.
4. Verificar pausa, logro y Códice; salir con Exit.
5. Recuperar carpeta completa logs/00.14-... después de60–120s; si el movimiento
   falla, indicar turno/acción y si avanza el turno pese a la imagen detenida.

runtime.log incluye control_state (casilla, sprite, edad, progreso de tween y destino).
crash-trace.log registra move_request, hero_goto y game_rejected_action.
performance.csv añade métricas de sombras. No se ha demostrado todavía la
causa del desajuste informado ni60FPS estables; véase [STATUS](STATUS_ITERATION14.md).
