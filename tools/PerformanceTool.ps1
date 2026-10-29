$ErrorActionPreference = 'Stop'
$pythonPath = Join-Path $env:USERPROFILE '.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe'
if (-not (Test-Path -LiteralPath $pythonPath)) {
    $pythonPath = (Get-Command python -ErrorAction Stop).Source
}
Add-Type -AssemblyName System.Windows.Forms
$dialog = New-Object System.Windows.Forms.OpenFileDialog
$dialog.Title = 'Abrir performance.csv de ShotgunKing2Vita'
$dialog.Filter = 'Perfil de rendimiento (*.csv)|*.csv'
if ($dialog.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) { exit }
$reportPath = Join-Path ([IO.Path]::GetDirectoryName($dialog.FileName)) 'ShotgunKing2Vita-performance.html'
& $pythonPath (Join-Path $PSScriptRoot 'perf_tool.py') report $dialog.FileName --output $reportPath
if ($LASTEXITCODE -ne 0) { throw 'No se pudo generar el informe.' }
Invoke-Item -LiteralPath $reportPath
