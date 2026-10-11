# Auditar estabilidad y recursos

En el repositorio o en el ZIP actualizado de la herramienta:

```sh
python tools/stability_audit.py private/game --output auditoria-nueva.json
```

En el ZIP, ejecutar `python stability_audit.py RUTA_DATOS --output auditoria-nueva.json`.
RUTA_DATOS es la carpeta preparada del juego o una copia local de los datos de
Vita. No usa el EXE/APK ni modifica imágenes o scripts. El archivo de salida
debe ser nuevo. Incluye fuente pública del runtime para comparar su interfaz.

La auditoría enumera nombres conocidos de SUGAR referenciados por los scripts,
incluso referencias capturadas como `bind(aspr,...)`. Marca funciones ausentes,
registradas y limitaciones conocidas. Es un inventario por tokens: wrappers,
variables locales y ramas condicionales requieren revisión; la presencia de
una función no demuestra compatibilidad completa.

Para cada SGI1 informa dimensiones, memoria de píxeles y estimación de tamaño
con deflate. La estimación no produce un formato nuevo ni lo instala: el
runtime sigue leyendo SGI1 sin compresión. Los 200 recursos preparados de la
copia examinada suman ~18,61 MiB de píxeles, aunque la sesión de juego no carga
todos a la vez (~1,74 MiB de superficies en la sesión recibida).

La compresión de almacenamiento no mejora por sí sola los FPS: al dibujar se
necesitan los mismos píxeles. Reducir dimensiones de atlas rompe las coordenadas
de sprites y fuentes. La escena ya es 320×180. Primero medir lógica, dibujo,
composición, copia a pantalla y espera; luego optimizar el coste dominante.

[Criterios de estabilidad y prioridades](INFORME_ROADMAP_ITERACION08.md).
