# Informe 00.16

Objetivo: separar el coste de presentación del coste de actualización/dibujo, probar una salida reducida reversible y observar los hilos del proceso antes de otra optimización.

Resultado esperado: menos bytes de escena y más trabajo de copia paralelo, manteniendo el orden de Lua y la sincronización GXM. La referencia física 00.14 tenía update 13,15 ms y draw 15,59 ms; presentación/subida era menor, por lo que esta iteración no promete 30/60 FPS.

Estado: compilación VitaSDK PASS; hardware pendiente. Vita3K sirve para arranque, controles y rutas de derrota/reinicio, con sus límites documentados.
