# ShotgunKing Vita
OJO TODO ES VIBECODED!!! no toque nada todo lo hizo chatgpt y su rama CODEX alimentandolo de datos a lo bestia!!!!!
## Publicación 00.38

La entrega numerada de la iteración 00.38 está en `release/00.38/` e incluye el VPK, `performance.cfg`, instrucciones y el diagrama de controles actualizado. Los datos comerciales permanecen fuera del repositorio; el extractor requiere una copia local legítima de Steam.




<img width="600" height="350" alt="image" src="https://github.com/user-attachments/assets/f4385ab2-c8b3-4f96-b9e7-ac9915adc8c3" />


La prueba simula botones y registra el estado. El benchmark presenta cada
fotograma sin VSync; sus tiempos/FPS derivados no son resultados de Vita.
La prueba de audio simula 300 segundos, verifica pista A→B, bucle, efectos y
silencio; guarda ocho segundos de PCM para inspección. No confirma salida audible
por el puerto de la consola.

## Investigación

El APK recibido contiene exclusivamente bibliotecas ARM64. El runtime usa los
scripts y recursos, sin cargar esos binarios. Hallazgos:
[viabilidad](docs/FEASIBILITY.md), [Steam](docs/STEAM_ANALYSIS.md),
[recursos GitHub](docs/GITHUB_RESOURCES.md).
La API se implementa a partir de la
[documentación pública SUGAR](https://github.com/TRASEVOL-DOG/Shotgun-King-Modding-Guide).
Dependencias y licencias: [third_party](third_party/README.md).

