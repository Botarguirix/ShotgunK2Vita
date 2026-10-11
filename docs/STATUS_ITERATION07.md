# Iteración 00.07 · stick derecho y triángulos

La sesión 00.06-1791694980-0 se detiene en el frame 774 con
`code.lua:594: attempt to call global 'trifill' (a nil value)`.
La inspección de piezas ROV dibuja flechas con esa función. La sesión termina
con `session_end=lua_error` tras salir; es una parada Lua recuperable.

El runtime implementa ahora `trifill()` y `tri()`, respetando cámara, recorte,
paleta, patrones y mezcla. Los bordes se recortan antes de recorrer píxeles.
El contrato procede del [manual público SUGAR](https://github.com/TRASEVOL-DOG/Shotgun-King-Modding-Guide/blob/main/SUGAR_manual.txt).

Validación:

- Antes del arreglo, el recorrido con stick derecho reproduce la misma función
  ausente en otra ruta, `code.lua:4031`, en el frame 3802.
- Después del arreglo, 5.400 frames con partida activa y giro continuo del stick
  derecho pasan en PC, también con AddressSanitizer/UndefinedBehaviorSanitizer.
- Cobertura de triángulos comparada con una referencia independiente: ambas
  orientaciones, cuatro flechas ROV, cámara, recorte, coordenadas grandes,
  patrones, paleta, mezcla, contornos y casos degenerados.
- Pasan las regresiones de repetición del pad y diagnósticos persistentes.
- Compilación Vita Release con símbolos. La prueba en consola sigue pendiente.

El usuario informa menores tiempos de carga. Antes del error, el CSV contiene
774 frames: media 49,536 ms (20,19 FPS derivados); los últimos 120 promedian
68,819 ms (14,53 FPS). Se excluye la pantalla detenida. El audio registra dos
bloques tardíos y ninguna pérdida de cola; no confirma sonido audible.

El core adjunto termina en `.tmp`, pero su gzip está completo y contiene un
ELF32 válido. Su nombre lo sitúa 523 segundos antes de esta sesión: se conserva
por separado y no se atribuye al error Lua ni a estos símbolos.

Instalar el VPK 00.07 sobre SGKV00001, conservando datos y saves. Probar stick
derecho en tablero e inspección de piezas, además de cruceta y stick izquierdo.
Si aparece otra parada, copiar la carpeta completa de la nueva sesión.
