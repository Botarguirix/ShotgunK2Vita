# VPK experimental 00.02

## Instalación

1. Extraer `shotgunk2vita-data.zip` en la computadora.
2. Copiar su carpeta `shotgunk2vita` a `ux0:data/` con VitaShell.
   Debe existir `ux0:data/shotgunk2vita/code.lua`, además de `code/`,
   `libs/`, `assets/` y los archivos `.sgimg` junto a sus PNG.
3. Copiar `ShotgunK2Vita-bootstrap.vpk` e instalarlo desde VitaShell.
4. Abrir ShotgunK2Vita Bootstrap desde LiveArea. Esperar la carga inicial.
5. X abre/acepta; cruceta o stick izquierdo navegan; círculo cancela;
   triángulo recarga; cuadrado activa acciones especiales; R dispara.
   El stick derecho apunta. START pausa. START+SELECT sale de la aplicación.
   Los gatillos de Vita comparten funciones de shoulder/trigger del mando PC;
   algunos menús necesitan adaptación adicional.
6. Elegir Throne, rank 1, Solomon y START. Avanzar las viñetas con X.

El ZIP se entrega sin partidas guardadas de la prueba de PC. No reemplazar
una carpeta `save/` propia si ya tienes progreso de una prueba anterior.
No necesita las bibliotecas ARM64 del APK, Steam, FMOD ni SDL3 de Windows.

## Evidencia y límites

La compilación nativa usa VitaSDK GCC 15.2.0. El VPK se valida como ZIP y
contiene eboot.bin y param.sfo. La prueba de PC llega a `ingame=true` y
`playing=true`, mostrando el primer tablero con los scripts originales.
Todavía no se ha medido ni verificado el funcionamiento en hardware Vita.

Audio y vibración están desactivados. CRT/window settings no se aplican.
El renderizador y la API SUGAR son parciales: pueden existir diferencias
visuales, de fuentes, paletas y texto, o llamadas pendientes en partidas,
cartas o modos que todavía no se han recorrido. No es un port completo.
Las partidas antiguas codificadas del juego original no se importan.

## Diagnóstico

Si aparece una pantalla de error, conservar
`ux0:data/shotgunk2vita/runtime.log`. Contiene el archivo/línea de Lua o
recurso que bloqueó el arranque y contadores de fotogramas nativos.
Enviar ese registro y describir hasta qué pantalla llegó el juego.
Si se cierra antes de escribirlo, informar también el código de error de Vita.
El registro se reemplaza en cada arranque: copiarlo antes de abrir otra vez.

Los datos del ZIP son archivos comerciales de la copia local del usuario.
No subir el ZIP, APK ni scripts/recursos del juego al repositorio.

Validación adicional: la misma secuencia de 4.800 fotogramas se ejecutó con
el ZIP extraído en una carpeta limpia, usando AddressSanitizer y
UndefinedBehaviorSanitizer en PC, sin diagnósticos. La prueba sin datos
termina con un error explícito de recurso faltante. Esto no verifica el
rendimiento, el heap o los servicios del sistema en Vita.
