# Iteración 00.27 — Codex Glyph Lookup

## Cambio

Las fuentes bitmap mantenían una búsqueda lineal por carácter en cada llamada
de texto y dibujo. 00.27 construye una tabla de índice de 256 entradas al
cargar cada fuente y reutiliza ese índice para avance y rasterización. La
salida permanece en 320×180 lógica con divisor 1; no cambia controles ni datos.

## Validación

- `sg_text_probe` compilado en Release con CMake host y ejecutado sin error.
- Se debe ejecutar la regresión común y la compilación Vita antes de instalar.
- No se declara una mejora de FPS hasta medir el mismo recorrido en Vita.

## Límite y prueba física

La optimización sólo afecta fuentes bitmap; las fuentes TTF conservan su caché.
En la Vita se debe comparar menú, Codex y tablero con PSVShell y conservar
`performance.csv` para la siguiente iteración.
