# 00.18 — Low Overhead Diagnostics

El log 00.17 mostró update aproximado de 2,6 ms, draw de 15,6 ms, subida de
0,65 ms y espera de presentación de 12–17 ms. El render sigue siendo el coste
dominante; reducir la resolución no era suficiente. Esta entrega deja el
perfilador desactivado por defecto para evitar escrituras y sincronizaciones
diagnósticas durante una partida. Se reactiva con `profile=1` para medir.

Los archivos originales no se sustituyen. El VPK contiene el ejecutable y la
configuración; los datos comerciales se conservan en `private/game` y se
instalan una vez en `ux0:data/shotgunk2vita`. En la Vita no se puede abrir
directamente un `data.sgr` de Windows como si fuera un directorio: el runtime
necesita sus recursos en la estructura de archivos Vita. La extracción puede
automatizarse con el preparador de datos, pero no debe duplicarse ni publicarse.
