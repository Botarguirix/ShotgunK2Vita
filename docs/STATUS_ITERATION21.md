# 00.21 — Native Resolution Controls

La configuración entregada fija explícitamente `resolution_divisor=1` para
evitar que una configuración antigua de 00.16 fuerce 160×90. Se conserva la
resolución nativa 320×180 y el escalado al framebuffer de Vita. La zona muerta
global de ambos sticks y la corrección `srand` permanecen.
