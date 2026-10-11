# Informe de roadmap · iteración 00.07

## Progreso confirmado

- El usuario confirmó arranque en OLED y mejora de FPS frente a 00.02.
- En 00.06 informa menores tiempos de carga. Sus logs identifican una parada
  Lua por `trifill()` al usar el stick derecho; [diagnóstico](STATUS_ITERATION07.md).
- 00.07 implementa triángulos. Pasan 5.400 frames con stick derecho en PC,
  sanitizadores, repetición del pad y diagnósticos. VPK validado y compilado.

## Estado actual

VPK 00.07 entregado; comprobación en Vita pendiente. La estructura histórica
de su entrega permanece intacta. Se estandariza el proceso de futuras entregas
siguiendo [ITERATION_PROCESS.md](ITERATION_PROCESS.md), inspirado en Touhou 7.5.

## Pendientes

Comprobar apuntado e inspección de piezas en hardware. Confirmar sonido audible,
estabilidad de partida y rendimiento en menús/tablero. Continuar compatibilidad
SUGAR, fidelidad gráfica y audio avanzado según la evidencia recibida.

## Siguiente prueba

Instalar 00.07 conservando datos/saves; mover ambos sticks, inspeccionar piezas,
navegar y disparar. Recoger toda la nueva sesión de logs y cualquier core nuevo.
No atribuir mejoras de FPS o estabilidad a 00.07 antes de esa prueba.
