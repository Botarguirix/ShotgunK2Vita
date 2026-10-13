# Pruebas locales y Vita3K

Usamos tres niveles: regresión PC con ASan/UBSan, arranque del VPK ARM en
Vita3K y comprobación final en PS Vita. Los FPS de PC/emulador no certifican
rendimiento, consumo ni sincronización en la consola.

Para campañas repetidas y comparación de mejoras, seguir
[OPTIMIZATION_ROUTINE.md](OPTIMIZATION_ROUTINE.md).

## Regresión local

En WSL, desde este proyecto: `bash scripts/check_iteration.sh`.
Cada ejecución conserva evidencia en una carpeta nueva. Comprueba movimiento
con cruceta/stick y posición final del sprite en dos partidas, combate,
apuntado, controles, logros, opciones y equivalencia de sombras.

## Emulador aislado

Desde PowerShell: `./tools/test_vita3k.ps1`.
Requiere Vita3K ya instalado, su firmware existente y `private/game` preparado.
Los parámetros permiten cambiar las rutas. Crea una carpeta única bajo
`reports/emulator/00.NN/`, copia el firmware y datos locales y registra el hash
del VPK. No modifica el perfil habitual ni las entregas anteriores.
Cierra otras sesiones antes: Vita3K puede compartir su log junto al ejecutable.
Datos y firmware permanecen fuera de Git.

El mapa de teclado se hereda de la configuración existente; confirmar primero
que CROSS abre el menú. Recorrer inicio, dos partidas, movimiento, disparo,
pausa, logros, L/R y Exit. Registrar por separado lo observado y lo pendiente.
Recuperar runtime.log, audio.log y performance.csv del storage aislado.

## Resultado del 13 de octubre de 2026, v14

- Regresión completa: PASS ASan/UBSan, `checks/run-Z85jXK`.
- Vita3K v0.2.1 4170-7e152b66, frontend SDL: instala y ejecuta el VPK v14;
  bootstrap correcto, portada animada y zonas L1/R1 visibles. Sesión
  `reports/emulator/00.14/storage/ux0/data/shotgunk2vita/logs/00.14-1791873131-0`.
- No se confirmó navegación por teclado: SPACE/Return no cambiaron la portada
  durante la observación. Jugabilidad, sonido audible y salida normal pendientes.
- Intentos Qt terminaron con `Failed to initialise config`; no califican como
  fallo del juego. El frontend SDL sí ejecutó el mismo VPK.
- El desajuste turno/sprite reportado en hardware sigue sin reproducirse.

Antes de limpiar, conservar logs, hash, configuración y resultado. La limpieza
final debe retirar sólo fixtures/cachés temporales identificados; mantener
evidencia, símbolos, snapshots y todas las entregas históricas. La prueba final
en Vita verifica FPS, audio, controles y ausencia del fallo visual.
