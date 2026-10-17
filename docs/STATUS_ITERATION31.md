# Iteración 00.31 — Integrated Candidate SFO Fix

00.30 no se instala en la Vita por un nombre de título demasiado largo para el
campo corto de `param.sfo`; la validación mostraba `STITLE` truncado. 00.31
mantiene el código integrado y acorta el nombre a `Integrated Candidate` para
producir un SFO compatible. La entrega 00.30 se conserva intacta como evidencia
del fallo.
