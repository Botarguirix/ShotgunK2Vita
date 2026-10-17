# Release 00.38

VPK: `ShotgunK2Vita-iteration38.vpk`

Cambios: zonas L1/R1 del touch trasero más pequeñas y centradas; deadzone del stick 40 con histéresis; touch frontal ignorado.

## Instalación

1. Instalar el VPK con VitaShell.
2. Conservar `ux0:data/shotgunk2vita/` y los saves existentes.
3. Copiar `performance.cfg` a `ux0:data/shotgunk2vita/performance.cfg` si se desea usar la configuración documentada.
4. Probar menú, tablero, granadas, Codex, L1/R1 traseros y salida por `Menu → Exit`.

## Datos del juego

El repositorio no distribuye APK, `data.sgr`, DLL, música ni partidas. Para preparar datos desde una copia legítima, usar:

```powershell
python tools/prepare_data.py "C:\ruta\juego.apk" private/game
python tools/prepare_livearea.py "C:\ruta\shotgun_king.exe" private/game private/livearea
```

La imagen de controles está en `docs/images/controls-vita-00.38.svg`. La evidencia y el informe completo están en `docs/STATUS_ITERATION38.md` e `docs/INFORME_ROADMAP_ITERACION38.md`.
