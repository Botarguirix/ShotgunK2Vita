# Iteración 11 — GPU Presenter · 00.11

## Evidencia recibida

Sesión 00.10-1791749926-0 conservada en reports/iterations/00.11/input.
1756 muestras activas: 17,80 FPS derivados, frame medio 56,178 ms y P95
83,372 ms. Update 9,248; draw 19,984; present 6,249; copy 8,598;
wait 9,378 ms. CPU observado 500 MHz, tres participantes. Sólo un parseo de
entrada: la caché funciona. Son recorridos distintos; no inferir regresión
respecto a la media de 00.09. DMA rc=0 falla integridad: fallback CPU correcto.

Error real: code/achievements.lua:455 llama fb_ev() inexistente después de
asignar DEN.achievements[id]. No falta el desbloqueo, falta la API de analítica.

## Cambios

fb_ev es un callback local sin envío de analítica. La lógica, popup y guardado
de logros continúan. No implementa trofeos PSN ni integración Steam.
La GPU recibe una textura RGBA de 320×180 (230400 bytes), la escala 3× por
vecino más próximo y centra verticalmente en y=2, conservando la paleta.
Antes se componía/copíaba 960×544 (2088960 bytes). El volumen por cuadro se
reduce 9,07 veces; esto no equivale a una mejora de FPS ya medida.
L1/R1 táctiles se dibujan con GXM. Se conserva SELECT+L/R y se consume el toque
de las esquinas. Sin contador propio en pantalla; las métricas siguen en logs.

Se usa el núcleo MIT de libvita2d, fijado a un commit y shaders incluidos.
Texturas y pool de comandos sin caché; fallbacks de memoria GPU sin caché.
La cola se completa antes de escribir otra vez la textura: evita modificarla
mientras se usa y mide cuadros presentados, incluyendo espera GPU/vblank.
GPU usa un participante CPU de composición; no crea workers de filas inútiles.
Lua permanece en CPU0 y audio conserva su hilo. No se afirma uso total de CPU.
renderer=1 (por defecto) GPU; renderer=0 conserva CPU anterior y sus workers.
Si la inicialización comprobada o la creación de texturas falla, se intenta CPU.
No se afirma cobertura completa de fallos internos de asignación de libvita2d.

CSV añade renderer (0 CPU / 1 GPU) y gpu_submit_ms. wait_ms incluye GPU/vblank;
copy_ms mide subida; present_ms composición pequeña. El informe reconoce ambos.
El shader/core tiene licencia/procedencia en third_party/libvita2d-core.

## Validación local

Regresión común ASan/UBSan PASS: entradas/caché, triángulos, apuntado, 6000
frames de combate con R, pausa real, logro y Exit, opciones, composición,
texto, repetición de pad, copia con corrupción simulada y diagnósticos.
Prueba específica en copia aislada: 00.10 reproduce fb_ev nil en la línea455;
00.11 desbloquea y escribe el ID del logro en save/achievements.sav.
La prueba de presentación comprueba píxel a píxel la composición 320×180
escalada 3× frente a 960×544, incluyendo la capa hdtext. GXM físico pendiente.
VitaSDK Release enlazado y VPK generado; CRC/SFO/LiveArea validados al empaquetar.
GPU no es ejecutable en el ensayo PC: falta medir imagen, estabilidad y FPS
reales en Vita. No se promete 30 ni 60 estables con esta entrega.

## Entrega y prueba física

artifacts/iterations/00.11/ShotgunK2Vita-iteration11-gpu-presenter.vpk.
Incluye símbolos, snapshot, evidencia, herramienta y hashes; anteriores intactos.
Instalar sobre SGKV00001 sin borrar ux0:data/shotgunk2vita ni save.
Recorrer título/menú/tablero 60–120 segundos, desbloquear un logro, revisar su
popup/lista después de reiniciar, probar L1/R1 táctiles, apuntado/disparo,
pausa/audio y Exit. Comprobar colores/texto, franjas y ausencia de flickering.
Enviar toda la nueva carpeta logs/00.11-... junto con los relojes del plugin.
Para comparar CPU, usar preset copy-cpu (renderer=0); repetir misma partida,
mismos relojes y escena. balanced restaura GPU. No forzar DMA corrupta.

Objetivo siguiente: acercar P95 a 33,3 ms para 30 FPS estables en juego;
60 necesita 16,7 ms. Sólo lógica+dibujo de esta sesión suman ~29,23 ms:
60 exige optimizar esas fases, además de la transferencia. 12 Frame Budget
perfilará las rutas dominantes después de recibir la medición GPU.
