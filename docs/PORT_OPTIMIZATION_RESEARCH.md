# Referencias de optimización y aplicación (2026-10-11)

Se consultaron fuentes primarias públicas; las siguientes aplicaciones son
propuestas para nuestro runtime y necesitan medición, no ganancias garantizadas.

| Proyecto / fuente | Técnica documentada | Aplicación en ShotgunK2Vita |
|---|---|---|
| [vitaGL](https://github.com/Rinnegatamante/vitaGL#readme) | sceGxm, profiling y opción NO_DMAC; DMA puede ser peor en algunos casos | 09 compara transferencias e integra fallback; 11 estudiar GPU |
| [vitaQuake](https://github.com/Rinnegatamante/vitaQuake#readme) | Renderizado hardware y benchmark | 11 presentación GPU; 12 recorridos reproducibles y presupuestos por fase |
| [libvita2d](https://github.com/xerpi/libvita2d) | Biblioteca 2D acelerada por GPU | 11 escalar la escena 320×180 mediante textura |
| [libvita2d: buffers](https://github.com/xerpi/libvita2d/blob/master/libvita2d/source/vita2d.c) | Tres buffers y cola de swaps GXM | Evaluar sincronización al implementar GPU, conservar protección de scanout |

Nuestra escena ya es 320×180 e imágenes indexadas de 8 bits. Reducir dimensiones
de atlas cambia coordenadas; comprimir archivos no elimina rasterización. El
siguiente beneficio esperado debe venir de reducir trabajo/copias y perfilar.
00.09 conserva rasterizado CPU y dos buffers; no incorpora todavía vitaGL/GXM.
La copia síncrona usa sceDmacMemcpy, declarada en VitaSDK psp2/kernel/dmac.h,
con biblioteca de usuario SceKernelDmacMgr_stub. El arranque verifica ocho
patrones cambiantes en RAM cacheada y registra ambas duraciones. Configuración
automática elige DMA sólo si pasa y resulta más rápida; en ejecución un error
negativo vuelve a memcpy. Mantener perfiles CPU/DMA para comprobar sesiones
reales: el benchmark inicial no garantiza mejora en todos los modos.

10 Input Cache: el coste de buscar/parsear cadenas de bindings puede evitarse
registrando una representación una vez. Verificar cruceta, sticks, combinaciones
y liberaciones; medir update_ms antes/después con idéntico recorrido.
11 GPU Presenter: subir textura de escena, escalar y combinar overlay por GPU;
verificar paleta, transparencia, texto, consumo de memoria y sincronización.
12 Frame Budget: perfilar Lua y primitivas más costosas, optimizar sólo las
rutas dominantes; comprobar media/P95 y partidas largas. Tres núcleos ya se
usan; Lua sigue en un hilo, repartir indiscriminadamente no reduce dependencia.
