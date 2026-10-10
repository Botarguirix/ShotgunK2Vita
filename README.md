# ShotgunK2Vita

Investigación de un port de Shotgun King a PS Vita. Estado: diagnóstico nativo;
el motor y el juego todavía no están integrados. Este VPK no ejecuta Shotgun King.

El APK recibido contiene únicamente ARM64; se descarta el so-loader directo
para ese archivo. Consulte la [evaluación de archivos](docs/FEASIBILITY.md)
para las dependencias comprobadas y alternativas de implementación.

## Compilar

Requiere VitaSDK, CMake y un entorno Linux/WSL.

```sh
export VITASDK=/home/bota/vitasdk
bash scripts/build.sh
```

Salida: `build/ShotgunK2Vita-diagnostic.vpk`, título `SGKV00001`.
Instalar con VitaShell. Mover el cursor con cruceta, stick izquierdo o pantalla
táctil; X cambia el cursor a verde; START sale. Un tablero de prueba y dos
indicadores verifican el framebuffer y la entrada táctil. El indicador inferior
se vuelve verde al tocar la pantalla. Registro: `ux0:data/sg2vita/diagnostic.log`.
Arranque y controles requieren verificación en hardware real.

## Archivos necesarios

Proporcionar las rutas al APK adquirido, todos sus splits (especialmente
`armeabi-v7a`) y OBB si existen; también a la instalación de Steam, con sus
bibliotecas, datos, scripts y log de arranque. Son distribuciones compiladas,
no necesariamente proyectos fuente. Guardar copias de trabajo en `private/`.
No publicar archivos comerciales en GitHub.

```sh
python3 tools/inspect_apk.py private/base.apk private/arm.apk --output reports/apk.json
```

El inspector lee cabeceras ELF y nombres; no determina compatibilidad completa.
Si hay ELF32 ARM: estudiar imports, JNI, ABI softfp, render, audio y entrada
antes de integrar un so-loader. Si solo hay ARM64, se requiere una distribución
ARM32 o el código fuente del motor; Vita no ejecuta directamente ARM64.
Una alternativa es adaptar SUGAR a Vita si existe acceso a su código fuente.
Los scripts Lua por sí solos no reemplazan el motor.

## Investigación (2026-10-10)

No se encontró un port de Shotgun King para Vita en las búsquedas realizadas.
La [guía del desarrollador](https://github.com/TRASEVOL-DOG/Shotgun-King-Modding-Guide)
confirma SUGAR y Lua en el juego. El motor exacto del APK queda por comprobar.
Existe una [edición Android oficial](https://play.google.com/store/apps/details?id=com.pid.shotgunking).
El [so-loader boilerplate](https://github.com/v-atamanenko/soloader-boilerplate)
es una base potencial, no un port listo ni garantía de compatibilidad.
La compilación usa las macros oficiales de [VitaSDK](https://github.com/vitasdk/samples).

## Validación de esta base

Compilado localmente con VitaSDK GCC 15.2.0. Se verificó la integridad ZIP del
VPK y la presencia de `eboot.bin` y `sce_sys/param.sfo`. El inspector distingue
cabeceras sintéticas ELF32 ARM y ELF64 AArch64. No se ha probado el arranque
en la consola ni integrado ninguna biblioteca del juego. El workflow de GitHub
genera un artefacto descargable; su ejecución se valida por separado.
