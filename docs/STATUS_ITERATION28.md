# Iteración 00.28 — Shadow Overdraw Budget

La ruta nativa `shpr` ahora recorta la región de sombra a los límites de la
superficie destino antes de leer la máscara y ejecutar operaciones de paleta.
Las sombras completamente fuera de pantalla dejan de consumir trabajo. La
semántica visible se conserva porque `dot()` ya descartaba esos píxeles.

Validación prevista: equivalencia de sombras en seis escenarios, presupuesto de
3.000 llamadas, regresión común, compilación VitaSDK y comprobación Vita3K.
La tasa de FPS real sólo se aceptará con medición en hardware.
