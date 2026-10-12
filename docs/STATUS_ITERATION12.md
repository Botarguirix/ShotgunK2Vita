# Iteración 12 — Frame Budget · 00.12

## Resultados de Vita 00.11

Entrada 00.11-1791751848-0 conservada privada en reports/iterations/00.12/input.
6089 muestras activas: 18,36 FPS derivados; media54,479ms, P95 83,382ms.
Update9,959ms, draw19,240ms, present12,577ms, copy/subida0,639ms,
envío GXM0,226ms, espera9,339ms. CPU observado500MHz. Exit termina limpio,
sin traceback Lua. El usuario reporta aviso de logro blanco. El título puede
rondar55FPS en tramos del log, pero la partida contiene caídas a14–18FPS;
no es 30/60 estable. Transferencia pequeña funciona; composición aún costosa.

## Interfaces locales e integraciones

Los logros locales se guardan en DEN.achievements y save/achievements.sav.
No son necesarios para iniciar/jugar, pero son parte del progreso y Códice;
se conservan, sin trofeos PSN ni sincronización Steam. Se ven en menú principal
→ Códice → pestaña Logros; R1 táctil o SELECT+R cambia a esa pestaña.
L1 táctil o SELECT+L vuelve a la otra. No borrar saves para actualizar.

No se encontró Twitch en los scripts Lua de estos datos. Sí hay Discord y
Privacy Policy como botones externos en el título. El cargador omite ese
bloque conocido en memoria, preservando saltos de línea y recursos originales.
No se distribuye código comercial parcheado ni hay que sustituir datos.
Si los marcadores cambian, el log registra pattern_not_found; no se pretende
soporte de otros paquetes. URL/Discord/Twitch tienen callbacks inertes; ya
estaban deshabilitadas Steam/PSN/Live y la analítica fb_ev. Créditos se conservan.

El aviso original se reproduce correctamente en PC en el menú; no se ha
reproducido exactamente el cuadro blanco de la partida física. Se sustituye
ach_unlock por un aviso propio, con fondo oscuro, texto blanco y borde,
compuesto después de la paleta y del overlay del juego. Evita esa ruta visual
sin tocar la condición de desbloqueo, guardado ni lista del Códice. Dura4s de
reloj real; cola acotada8 avisos. Textos largos ocupan dos líneas y se recortan;
acentos latinos se normalizan en el bitmap, otros caracteres no están cubiertos.
No afirmar corrección física antes de recibir la prueba de Vita.

## Rendimiento

Composición de tamaño lógico usa un bucle directo y precomputa coordenadas de
la capa hdtext. Evita la división y expansión genérica por cada píxel.
La ruta CPU de960×544 y la GPU320×180 siguen disponibles; aviso propio en ambas.
Ensayo PC ASan/UBSan de400 composiciones:393951→196798µs, hash6b6e7989 idéntico.
Aproximadamente50% menos en esa prueba, sin predecir FPS ni coste ARM real.
El presupuesto de30FPS es33,3ms; update+draw aún suman29,2ms. 60 necesita16,7ms
por cuadro y exige reducir dibujo/Lua además de presentación.

## Validación y entrega

Compilación VitaSDK Release completa; SFO00.12, ZIP/CRC y LiveArea válidos.
Regresión común ASan/UBSan PASS. Aviso con9064 píxeles oscuros y374 de texto;
entrada a Códice y cambio R1 a Logros PASS, igual que desbloqueo/guardado,
combate de6000frames, pausa, Exit, opciones, presentación, pad y diagnósticos.
Resultados conservados en evidence/result.txt y local-ui.log.
La prueba comprueba contraste del aviso, entrada a Códice y cambio a Logros
con R1; desbloqueo y guardado permanecen en la regresión de logros.
Imagen equivalente sin aviso verificada por hash y prueba serial/paralela.
Hardware pendiente.

artifacts/iterations/00.12/ShotgunK2Vita-iteration12-frame-budget.vpk.
Carpeta independiente con símbolos/snapshot/evidencia/herramientas/hashes.
Instalar sobre SGKV00001 conservando ux0:data/shotgunk2vita y save.
Probar desaparición de enlaces externos, nombre legible del logro al finalizar
un piso, Códice/Logros tras reiniciar, R1/L1 táctiles, pausa, audio y Exit.
Medir título y misma partida durante60–120s con mismos relojes; enviar carpeta
logs/00.12-... completa y captura si el aviso sigue incorrecto.

Próxima13 — Draw Budget: perfilar primitivas y callbacks dominantes en la partida
una vez cuantificada la reducción real de present_ms. Mantener media/P95,
controles, audio y estabilidad; no compresión de archivos como sustituto de
reducir trabajo de dibujo. No HUD propio.
