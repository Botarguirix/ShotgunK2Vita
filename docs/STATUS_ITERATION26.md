# 00.26 — GPU Overlay Cleanup

Se elimina la creación y liberación de las texturas de los antiguos botones
frontales L1/R1. Ya no se dibujan desde el presentador y L1/R1 llegan por el
touch trasero. La escena conserva resolución y sincronización; esta pasada
reduce recursos GPU de inicialización y composición.
