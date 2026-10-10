# Análisis estático de Steam

El ejecutable y las tres DLL recibidas son PE x86 de 32 bits (máquina 0x14c).
La dirección preferida de entrada del EXE es 0x4010ba, base 0x400000.
No se ejecutaron el juego ni los lanzadores.

Los BAT solo lanzan `shotgun_king.exe`, con el argumento `shaderless` para
el segundo. Ese argumento no demuestra que el motor funcione sin SDL/OpenGL.

El EXE importa SDL3, OpenGL32, Steam API, Discord SDK, Kernel32, User32,
Shell32 y bibliotecas CRT. No importa una DLL Lua independiente. La cadena
`Lua 5.1` aparece dentro del ejecutable; es indicio de runtime incorporado.

Puntos de referencia para el análisis del cargador (direcciones preferidas):

| Cadena | Dirección |
|---|---|
| data.sgr | 0x719126 |
| Failed to uncompress export data with error code | 0x7194d4 |
| newsrf | 0x711fc8 |
| newwin | 0x712003 |
| Lua 5.1 | 0x73b9c8 |

Hay cadenas relacionadas con inflate y una versión zlib. Esto sugiere
descompresión en el motor, pero no identifica todavía el formato de data.sgr,
el algoritmo exacto ni una transformación previa de sus bytes.

El EXE tiene secciones con nombres largos sin resolver y cadenas de tipos
y archivos fuente. Su puntero/cantidad COFF declarados exceden el tamaño
del archivo al calcular la tabla de cadenas: no se consideran símbolos válidos.
Queda pendiente comprobar la información de depuración con un analizador
capaz de interpretar estas secciones. El objdump disponible en WSL rechazó
el formato de archivo; no se obtuvo desensamblado con esa herramienta.

`tools/inspect_pe.py` genera un informe local de cabeceras, imports, exports,
secciones y cadenas relevantes en `reports/steam-pe.json`. Los informes
y archivos comerciales no se publican.

Siguiente paso: importar el EXE en Ghidra como PE x86, comprobar secciones
y referencias a las cadenas del cargador; seguir lectura de archivo,
transformación y descompresión. Después localizar el registro de funciones
Lua para reproducir la API necesaria en Vita. No se ha decompilado todavía
ninguna de esas funciones ni recuperado el código fuente original.
