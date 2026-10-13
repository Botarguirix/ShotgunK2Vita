# 00.15 — Game Over Recovery

Evidencia recibida: `00.14-1791874874-0`, conservada en
`reports/iterations/00.15/input/`. El usuario reporta parada después de perder.
Es un error Lua protegido, no hay un core nativo en esta entrega de logs:
`code.lua:11556 check_folly_shields`, desde is_safe/shoot y un botón del tablero.
Frame 7665, CROSS activo, hero.sq ya ausente. Sale después con START.

## Diagnóstico y corrección

El iterador nativo all avanzaba siempre al índice siguiente. del desplaza los
elementos de la lista: al borrar el actual se omitía el nuevo elemento en ese
índice. remove_buts usa esta combinación para retirar controles del tablero.
La reproducción PC dejó 18 botones después de retirarlos y el disparo tras
`xpl(hero)` reprodujo la misma excepción check_folly_shields de v14.
Es una reproducción de las condiciones y ruta de error, no una repetición
exacta de todos los inputs de la sesión física.

all usa ahora cursores que add/del ajustan cuando desplazan posiciones. Los
cursores se desvinculan al terminar; __gc limpia los bucles abandonados. No se
copia toda la lista en cada all ni se cambia foreach en esta iteración.
Comprueba duplicados, borrado del actual/anterior/futuro, inserciones, bucles
anidados y finalización por GC. La prueba real del tablero deja cero botones.

Se añade una protección al cargar `shoot(wild, sq)`: vuelve sin actuar si hero
no existe, está muerto o no tiene casilla. Evita callbacks tardíos durante la
transición; no altera comprobaciones de peligro ni permite disparar sin casilla.
El parche se aplica en memoria; no exige sustituir datos del juego en la Vita.
Mantiene los números de línea de los scripts y registra su aplicación.

## Rendimiento recibido

7665 muestras activas anteriores a la parada: 22,21 FPS de sesión mixta,
media45,02ms y P95 66,74ms. Update13,15ms, draw15,59ms; CPU500MHz, GPU,
un participante de presentación. Shadow fallback0. No comparar directamente
con las sesiones anteriores: no contienen exactamente las mismas escenas.
No se cuentan los aproximadamente55FPS de la pantalla de error como juego.

## Validación y entrega

Reproducción anterior: `repro-old.log`, OLD_GAMEOVER_ERROR=1, BUTTONS_LEFT=18.
Pruebas focalizadas: MUTATION_SAFE_ITERATION=1 y GAMEOVER_RESTART=1;
BUTTONS_LEFT=0, 300 frames con CROSS/R después de morir y nueva partida con
movimiento y sprite sincronizados. Regresión completa ASan/UBSan PASS:
`checks/run-GPNsbq/result.txt`. VitaSDK Release, SFO00.15, ZIP/CRC y LiveArea
PASS. Aceptación física pendiente.

Campaña Release PC: 18/18 PASS, `performance-campaign/`, huellas equivalentes
entre participantes, pases y referencia14. Cinco comparaciones quedan
inconclusas por variación; tablero con3 participantes es candidato (~22,2%
menos media PC, ~29,1% menos P95). Es una comparación de muestras en este PC,
no prueba causal del arreglo ni predicción de FPS en la Vita. Se preservan todos
los pases y los fallos anteriores; no se seleccionan sólo los mejores tiempos.

Carpeta nueva `artifacts/iterations/00.15/`, VPK
`ShotgunK2Vita-iteration15-game-over-recovery.vpk`, símbolos, fuente y evidencia.
Instalar conservando ux0:data/shotgunk2vita y saves. No sobrescribe v14.

Probar perder, pulsar CROSS/R durante la animación de Game Over, elegir volver
a jugar y completar otra partida. Revisar fin de juego, retorno al menú y Exit.
Recuperar logs/00.15-... para confirmar estabilidad y medir escenas físicas.
El fallo intermitente de sprite informado anteriormente sigue en seguimiento;
esta corrección elimina controles sobrantes, pero no prueba su causa completa.
