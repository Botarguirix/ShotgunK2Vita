# Iteración 09 — Frame Transfer · 00.09

## Evidencia y causa

Entrada privada preservada: sesión 00.08-1791698759-0 y captura recibida.
La captura/log señalan code.lua:11497: ismobile() nil, en el menú de pausa.
La plataforma mantiene MOBILE=false y PC=true para los controles existentes.
Registro ismobile() devuelve false de manera coherente con esa ruta.
La regresión reveló además OPTIONS[id] numérico tras abrir la pausa. El runtime
convierte alias numéricos por id a los mismos descriptores, conservando la lista y SET.
7036 frames activos: 18,00 FPS, media 55,56 ms, P95 83,42 ms; copy 8,41 ms.
Se excluye la pantalla detenida. No comparar directamente medias de otro recorrido.

## Cambio

HUD retirado de main: no hay dibujo de contador ni reactivación por configuración
antigua o combinación. Métricas compiladas: U/D/present/copy/wait, FPS en log,
CSV y memoria. CPU/DMA se comparan sobre el framebuffer inactivo al arrancar,
con ocho patrones variables y memcmp completo; selección automática, perfiles
forzados y fallback CPU por retorno negativo. Copia síncrona antes del swap;
doble buffer y confirmación de scanout conservados. CSV añade copy_mode/copy_rc.
Nombre central en CMake; SFO/iteration.json y archivo VPK identifican la entrega.

## Validación y límites

PASS: regresión común con ASan/UBSan, ampliada a START en frames 5000/5100 de combate;
recorrido anterior no alcanzó la pausa; se fuerza la ruta real pause_game
y se reproduce el contrato ausente quitando ismobile en el harness PC. Resultados y logs originales
quedan en evidence, junto a compilación Vita y validación CRC/SFO/LiveArea.
Prueba de copia host con DMA simulado: rechaza no-op/corrupción, valida
selección forzada y fallback con igualdad de bytes, sin avisos ASan/UBSan.
6000 frames y 12 entradas R completados; ACTUAL_PAUSE_ROUTE=passed.
Pruebas de stick, triángulos, opciones, presentación, texto, pad y logs pasan.
Vita Release compilado, CRC/SFO/LiveArea válidos y harness PC ausente del ELF.
El código nativo DMA sólo puede comprobarse completamente en hardware; su
ensayo de arranque está integrado, pero no se afirma más FPS hasta recibir CSV.
Compatibilidad de todos los modos/cartas, saves antiguos y audio avanzado
continúa pendiente; no se declara una versión estable definitiva.

## Entrega / prueba de aceptación

artifacts/iterations/00.09/ShotgunK2Vita-iteration09-frame-transfer.vpk.
Instalar sobre SGKV00001 conservando ux0:data/shotgunk2vita/ y save/.
Probar START/pausa/opciones, R, recarga y sticks durante cinco minutos; no debe
aparecer ismobile nil ni nuestro HUD. Comparar copy-cpu/copy-dma en el mismo
recorrido. Enviar carpeta completa logs/00.09-*; confirmar imagen sin parpadeo,
FPS con plugin y audio. Las entregas anteriores permanecen intactas.
