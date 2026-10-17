# ShotgunKing Vita

## Publicación 00.38

La entrega numerada de la iteración 00.38 está en `release/00.38/` e incluye el VPK, `performance.cfg`, instrucciones y el diagrama de controles actualizado. Los datos comerciales permanecen fuera del repositorio; el extractor requiere una copia local legítima del APK o Steam.




<img width="600" height="350" alt="image" src="https://github.com/user-attachments/assets/f4385ab2-c8b3-4f96-b9e7-ac9915adc8c3" />




## Compilar

Requiere VitaSDK, CMake y Linux/WSL. Lua, stb_truetype y dr_mp3 están incluidos.

```sh
export VITASDK=/home/bota/vitasdk
bash scripts/build.sh
```

## Pruebas en PC

```sh
cmake -S . -B build-host -DSG_HOST_PROBE=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-host -j4
mkdir -p reports
./build-host/sg_probe private/game 4800 game
./build-host/sg_bench private/game reports/perf.csv 4800 reports/frame.ppm 3
./build-host/sg_audio_probe private/game reports/audio.wav
```

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

