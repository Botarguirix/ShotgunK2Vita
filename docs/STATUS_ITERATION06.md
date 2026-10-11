# Iteración 00.06 · repetición de pad

El `runtime.log` aportado registra la parada al mantener una dirección:
`gamepad.lua:1994: attempt to call global 'delta' (a nil value)`.
La función `btnrp()` llama a `delta()` después de la primera pulsación para
contar el retraso de repetición. El runtime había registrado `dt()` pero omitió
su alias `delta()`. La implementación se comparte ahora entre ambos nombres,
como indica el [manual SUGAR](https://github.com/TRASEVOL-DOG/Shotgun-King-Modding-Guide/blob/main/SUGAR_manual.txt).
Conserva el paso fijo actual de 1/60 s; esta entrega no cambia el reloj de la
simulación a tiempo real ni ajusta el reloj de CPU.

El log confirma 21–29 FPS antes del fallo, presentación en núcleos 0/1/2 y
CPU real 500 MHz, aunque se solicitaron 444. No confirma audio audible ni
estabilidad de una partida completa. Su cabecera `performance-00.03` corresponde
al runtime que compartían 00.03 y 00.04; no es el identificador de 00.05.

Validación:

- Con la implementación anterior y `gamepad.lua` de los datos originales,
  reproducido el mismo error en la línea 1994 al segundo frame mantenido.
- Con el arreglo, prueba de 1.440 frames mantenidos: cuatro direcciones de
  cruceta y stick, tres ciclos con liberación entre cada entrada. Verifica
  repetición y que ambos nombres de la API devuelven el mismo valor positivo.
- Se conservan las pruebas de controles, logs persistentes, recorrido hasta
  partida activa y validación del VPK.

Se centraliza la versión en `SG_BUILD_VERSION` de CMake: el SFO, encabezado
de log, carpeta de sesión, compilación y entrega deben identificar 00.06.
Las entregas anteriores permanecen intactas. Datos y saves de Vita no necesitan
reemplazo. La prueba en hardware del arreglo sigue pendiente.

En la siguiente prueba: mantener cruceta/stick en menús y tablero, soltar y
volver a pulsar; copiar `ux0:data/shotgunk2vita/logs/` si aparece otra parada.
