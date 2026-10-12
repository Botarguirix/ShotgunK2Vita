# Roadmap — Iteración 12 Frame Budget

11 confirmada en Vita: arranque GPU y Exit, subida0,639ms; 18,36FPS globales
activos, P95 83,38ms. Continúan caídas en partida y aviso de logro blanco.

12: optimizar composición lógica, retirar enlaces externos del título y
sustituir aviso por bitmap local. Conservar progreso, Códice y saves.
Composición PC hash idéntico y coste aproximadamente50% menor.
Regresión ASan/UBSan, aviso legible y Códice con R1 PASS; hardware pendiente.

13 — Draw Budget: comparar present_ms en misma partida; perfilar dibujo/Lua
(~29,2ms en11) y optimizar sus rutas dominantes. Objetivo30FPS con P95 cercano
al presupuesto33,3ms y sesiones sin fallos. 60 todavía requiere bajar de16,7ms.
Prueba pendiente: logro legible y persistente, Códice con R1, partida y audio.

[Detalles, límites y entrega](STATUS_ITERATION12.md). Entregas previas intactas.
