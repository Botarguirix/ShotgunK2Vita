# Iteración 00.32 — Custom Loading Screen

Se sustituye el texto de arranque `LOADING LUA AND GAME DATA` por una pantalla
propia de Shotgun King Vita: fondo oscuro, marco, título, subtítulo, barra de
progreso y estado de espera. Se usa tanto en el backend GPU como en el fallback
de framebuffer. Sólo se ejecuta una vez durante el arranque.

La investigación de Lua concluyó que LuaJIT Vita es una opción experimental,
pero migrar el runtime no es un cambio seguro sin probar compatibilidad con la
API SUGAR y el recolector. Se conserva Lua 5.1 mientras se mide una rama
separada.
