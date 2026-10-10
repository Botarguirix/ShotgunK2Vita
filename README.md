# ShotgunK2Vita

Port experimental de Shotgun King a PS Vita. El runtime propio interpreta los
scripts Lua del APK con Lua 5.1 y una implementación parcial de la API SUGAR.
Dibuja superficies indexadas, texto, sprites y rotación mediante software.

La prueba de PC alcanza el menú original y una partida activa de Throne.
El VPK nativo se compila con VitaSDK; falta probarlo en una Vita física.
Esta versión no tiene audio, CRT ni compatibilidad completa con todos los modos,
partidas guardadas anteriores, idiomas y funciones del motor original.

## Probar en Vita

Ver [instrucciones del VPK de prueba](docs/TEST_BUILD.md).
Los datos comerciales se preparan localmente y no se publican en este repositorio.

## Compilar

Requiere VitaSDK, CMake y Linux/WSL. Lua y stb_truetype están incluidos.

```sh
export VITASDK=/home/bota/vitasdk
bash scripts/build.sh
```

Salida: `build/ShotgunK2Vita-bootstrap.vpk`, título `SGKV00001`, versión 00.02.
El workflow también compila este VPK, sin incluir los datos del juego.

## Preparar datos de tu APK

Requiere Python 3.9+ y Pillow. El preparador extrae recursos y scripts;
no ejecuta ni distribuye las bibliotecas nativas del APK.

```sh
python3 tools/prepare_data.py /ruta/juego.apk private/game
python3 tools/package_test.py
```

La segunda orden genera el VPK, ZIP de datos y sus hashes en `artifacts/`.
Cada PNG recibe un archivo `.sgimg` auxiliar para el renderizador indexado.
No funciona copiando únicamente `data.sgr` de Steam.

## Prueba del runtime en PC

```sh
cmake -S . -B build-host -DSG_HOST_PROBE=ON
cmake --build build-host -j4
mkdir -p reports
./build-host/sg_probe private/game 4800 game
```

La prueba simula botones a intervalos fijos, registra el estado de la partida
y genera `reports/host-frame.ppm`. No equivale a una prueba en hardware Vita.

## Investigación

El APK recibido contiene exclusivamente bibliotecas ARM64. El runtime Vita
usa los scripts y recursos; no carga esos binarios. Referencias y hallazgos:
[viabilidad](docs/FEASIBILITY.md), [Steam](docs/STEAM_ANALYSIS.md),
[recursos GitHub](docs/GITHUB_RESOURCES.md).

La API se ha implementado a partir de la
[documentación pública SUGAR](https://github.com/TRASEVOL-DOG/Shotgun-King-Modding-Guide).
Dependencias y licencias: [third_party](third_party/README.md).
