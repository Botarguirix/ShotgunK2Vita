# 00.19 — Ability Random Seed Guard

El log 00.18 terminó al usar una habilidad: `read_target_mind` llamó a la
global Lua `srand`, que no estaba registrada por el runtime Vita. La ruta se
reproduce en `code.lua:10722` y aparece como `attempt to call global 'srand'`.

Se añade el puente nativo `srand(seed)` junto a `rnd` e `irnd`, usando el
generador C que ya emplea el runtime. La semántica esperada por la habilidad se
conserva y el resto de errores Lua sigue protegido y visible.
