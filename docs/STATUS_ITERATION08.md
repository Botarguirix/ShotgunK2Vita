# Iteración 00.08 · disparo, opciones y presentación

## Evidencia y causa

Sesión recibida `00.07-1791696912-0`: `libs/toolkit.lua:456: attempt to call
upvalue 'f' (a nil value)`. La traza pasa por `code.lua:8592` y `brd()`.
El casquillo del disparo usa `bind(aspr,...)`: faltaba `aspr()` al capturar
la función. Antes del arreglo, la prueba en PC reproduce esa misma traza
en el frame 4001 con la primera entrada R.

La sesión anterior `00.07-1791696864-0` registra `bget()` ausente en
`code/data.lua:735`, al modificar un slider de opciones. Este APK pasa un
descriptor OPTIONS a esa llamada; se añade un puente hacia el campo `SET[nid]`
y su valor predeterminado. No es soporte completo para bancos numéricos.

Antes de la parada, 940 muestras de Vita promedian 51,196 ms: 19,53 FPS
derivados. Update 7,695 ms, dibujo 16,679 ms, presentación 14,538 ms y espera
9,130 ms. CPU observada 500 MHz; tres participantes en presentación. Se
excluyen los frames posteriores al primer checkpoint `running=0`, que no
representan juego activo. Audio registra cinco bloques tardíos, máximo de
mezcla 111047 µs, ninguna pérdida de cola; no demuestra sonido audible.

## Corrección y optimización

- `aspr()` reutiliza la rotación de `asspr()`, incluyendo dimensiones de
  sprites fraccionarias para el casquillo. `circ()` añade un contorno acotado.
- Los sliders con descriptor actualizan SET; coordenadas de bancos antiguos
  se rechazan explícitamente. Importación de saves antiguos continúa pendiente.
- Doble framebuffer CDRAM: no se dibuja sobre el buffer visible. Composición
  y lecturas de filas repetidas en RAM cacheada, copia completa al buffer
  inactivo, cambio a NEXTFRAME y comprobación del buffer activo tras VBlank.
  Es una corrección de la escritura sobre scanout y una posible explicación
  del parpadeo informado; falta comprobarlo en la pantalla de Vita.
- Texto con máscaras de glifos recortadas antes de recorrer píxeles y ruta
  rápida para paleta sin patrones/mezcla. Se conservan esas operaciones cuando
  están activas. La imagen final del recorrido de 3900 frames coincide con
  00.07, y la prueba específica de texto conserva el hash.
- CSV añade `copy_ms` y `running`. El informe excluye muestras detenidas y
  separa el coste de copia. La pantalla de error no muestra FPS de juego.

No se bajó la resolución lógica: ya es 320×180, escalada a 960×544. Los atlas
usan índices de 8 bits. Comprimirlos puede reducir almacenamiento y cambiar
carga/descompresión, pero no reduce el número de píxeles dibujados cada frame.
Cambiar dimensiones de atlas invalidaría coordenadas del juego.

## Comprobaciones

- Después del arreglo, 6000 frames con giro de stick, 12 entradas R y recarga;
  termina con partida activa. No equivale a completar una campaña.
- Regresiones de 5400 frames con stick derecho, triángulos, pad mantenido,
  opciones y persistencia de logs. ASan/UBSan sin diagnósticos.
- Presentación serial y 1/2/3 participantes: mismo framebuffer, escala y
  overlay, con guardas de memoria. Los hilos host no prueban el scanout Vita.
- Texto: 5000 repeticiones, hash idéntico `92ba0c756c366e65`. En esa medición
  PC pasa de 43470 a 29892 µs totales (~31% menos). No predice FPS de Vita.
- Compilación Vita Release con ELF/map. VPK se valida antes de empaquetar.
- Auditoría de APIs/imágenes y pruebas del filtrado del informe de rendimiento.

## Entrega y prueba pendiente

SGKV00001, versión 00.08, en `artifacts/iterations/00.08/`. Incluye VPK,
símbolos, snapshot, evidencia, herramientas y auditoría de entrega.
Instalar sobre el mismo título conservando datos y saves.

Probar disparo R, recarga, inspección con stick derecho y sliders de música/SFX.
Comprobar parpadeo tanto con HUD como sin él. Comparar perfiles balanced y
single en el mismo recorrido; conservar toda la sesión. Verificar música,
efectos y estabilidad más allá del primer tablero. FPS y parpadeo de 00.08
en hardware siguen pendientes.

Contratos: [aspr y SUGAR](https://github.com/TRASEVOL-DOG/Shotgun-King-Modding-Guide/blob/main/SUGAR_manual.txt),
[framebuffers y sincronización VitaSDK](https://docs.vitasdk.org/group__SceDisplayUser.html).
