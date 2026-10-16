# Comparación de candidatos 00.26–00.30

| Versión | Cambio | VPK (bytes) | Evidencia |
|---|---|---:|---|
| 00.26 | Limpieza overlay GPU | 387965 | Controles/touch local |
| 00.27 | Índice de glifos Codex | 387274 | `sg_text_probe`, regresión de entrada |
| 00.28 | Recorte de sombras fuera de pantalla | 387275 | 6 equivalencias; 110364→4634 µs en PC |
| 00.29 | Ruta rápida de silencio | 387488 | 300 s simulados; 0 fallos, 0 drops |
| 00.30 | Integrado | pendiente | compilación y Vita3K pendientes de esta entrega |

La variante recomendada para la prueba física es 00.30. Si el hardware revela
una regresión concreta, 00.28 y 00.29 permanecen disponibles como rollback.
