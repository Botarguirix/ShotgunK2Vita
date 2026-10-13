# Seguimiento — 15 Game Over Recovery

El log14 muestra un disparo desde controles del tablero después de que hero.sq
dejara de existir. Se reprodujo en PC y se corrigió la eliminación durante all:
remove_buts pasó de 18 controles sobrantes a cero. Protección adicional para
disparos tardíos con rey muerto/sin casilla. No se modifican archivos privados
en el repositorio público ni se requieren nuevos datos en consola.

La próxima prueba física debe cubrir derrota, botones durante la animación,
reintento, segundo fin de juego, menú y salida. Confirmar con logs15; pruebas
PC y compilación estática no equivalen a aceptación en la PS Vita.

Regresión ASan/UBSan, compilación VitaSDK y validación estática PASS. Campaña
local18/18 con imágenes equivalentes: mayoría de comparaciones inconclusas;
una escena queda como candidata, sin atribuir ganancia física. Evidencia y
símbolos acompañan el VPK independiente15.

El rendimiento14 fue22,21FPS en sesión mixta; no prueba una regresión respecto
a13 porque la carga difiere. Continuar campañas por escenas y medir update/draw
después de corregir los controles sobrantes. Objetivo60FPS; no se promete una
ganancia por este arreglo. Mantener estabilidad antes de añadir otra optimización.

Siguiente: 16 Lua Budget, condicionada a datos15. Investigar callbacks dominantes
y cargas de tablero avanzado; comprobar sprite/casilla y logros en partidas
completas, conservando las entregas y evidencia histórica.
