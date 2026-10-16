# Iteración 00.29 — Audio Presentation Budget

El mezclador nativo consume ahora la cola de comandos y, cuando no hay música
ni efectos activos, limpia el bloque de salida directamente. Antes recorría
todos los frames y los 12 canales aun durante silencio. La ruta activa conserva
la mezcla, interpolación, paneo y fades existentes.

Validación: prueba de audio de 300 segundos, silencio y efectos, regresión
común, compilación VitaSDK y arranque Vita3K. La tasa de FPS y la audición real
en PS Vita siguen requiriendo prueba física.
