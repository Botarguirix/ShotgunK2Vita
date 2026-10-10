# Recursos para ShotgunK2Vita

Investigación: 2026-10-10. No se encontró un port completo, decompilación del
motor ni extractor confirmado de data.sgr en las búsquedas realizadas.

| Recurso | Uso previsto | Límite |
|---|---|---|
| [Manual SUGAR](https://github.com/TRASEVOL-DOG/Shotgun-King-Modding-Guide/blob/main/SUGAR_manual.txt) | Contrato de funciones y callbacks | Versión documentada 0.0.6b; puede diferir del motor actual |
| [Sugarcoat](https://github.com/TRASEVOL-DOG/sugarcoat) | Referencia de comportamiento de parte de SUGAR | Usa LÖVE/Castle; no es un runtime Vita listo |
| [vita-luajit](https://github.com/hyln9/vita-luajit) | Intérprete LuaJIT en Vita | Falta toda la API SUGAR y verificar compatibilidad de los scripts |
| [lpp-vita](https://github.com/Rinnegatamante/lpp-vita) | Lua, gráficos, entrada y audio ya integrados | Su API difiere de SUGAR; necesita capa de adaptación |
| [libvita2d](https://github.com/xerpi/libvita2d) | Backend GPU para superficies y sprites | No reproduce por sí solo paletas, shaders ni semántica SUGAR |
| [Ghidra](https://github.com/NationalSecurityAgency/ghidra) | Localizar cargador SGR y funciones nativas | Produce análisis/pseudocódigo, no el proyecto fuente original |

Antes de incorporar código se revisará la licencia de cada componente.
La ruta inicial propuesta es un runtime propio Lua + backend 2D Vita,
usando manual/Sugarcoat como referencias. LPP-Vita es una alternativa para
un prototipo, no una compatibilidad automática. No se presupone un port
completo de LÖVE en Vita.

## Inventario preliminar

Se compararon los 34 scripts Lua del APK con 196 nombres encontrados en el
manual mediante `tools/map_sugar_api.py`: 130 nombres coincidentes. Incluyen
all, add, sspr, sfx, pal, rnd, flr, spritesheet, btnp, spr y rectfill.

Es una heurística textual: puede incluir definiciones/cadenas y omitir llamadas
indirectas o alias. No significa que haya exactamente 130 funciones pendientes
ni mide compatibilidad. El informe completo está en reports/sugar-api-map.json,
excluido de Git para conservar los datos de la instalación en local.

Siguiente hito: clasificar las coincidencias por arranque, menú, gráficos,
entrada, audio y guardado; verificar firma y semántica antes de implementar.
