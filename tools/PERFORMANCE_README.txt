ShotgunK2Vita Performance Tool · 00.14

Copiar sesión completa logs/ al PC tras salir mediante menú/Exit.
Abrir PerformanceTool.cmd y seleccionar performance.csv; requiere Python3.
El informe HTML funciona sin conexión. Conservar datos y save.
balanced/battery usan GPU; copy-cpu/single comparan renderizador CPU anterior.
workers sólo afecta CPU, no GPU. Evitar forzar DMA:falló integridad en consola.
Comparar misma escena/relojes y P95. Los microbenchmarks PC no predicen FPS Vita.
14 muestra contadores de entidades y sombras. Movimiento/animación se registra
en runtime y crash-trace. No HUD ni atajos SELECT; sólo L1/R1 usan frontal.
