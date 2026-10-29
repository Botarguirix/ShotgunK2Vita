param(
    [Parameter(Mandatory)][string]$Emulator,
    [Parameter(Mandatory)][string]$ExistingStorage,
    [Parameter(Mandatory)][string]$ExistingConfig,
    [Parameter(Mandatory)][string]$Vpk,
    [Parameter(Mandatory)][string]$DataRoot,
    [switch]$Steam,
    [string]$ReplayInput,
    [string]$ReplaySnapshot,
    [switch]$Interactive
)
$ErrorActionPreference = 'Stop'
foreach ($sgPath in @($Emulator,$ExistingStorage,$ExistingConfig,$Vpk,$DataRoot)) {
    if (!(Test-Path -LiteralPath $sgPath)) { throw "Missing input: $sgPath" }
}
if ([bool]$ReplayInput -ne [bool]$ReplaySnapshot) { throw 'ReplayInput and ReplaySnapshot must be supplied together.' }
$sgProject = Split-Path $PSScriptRoot -Parent
$sgRun = Join-Path $sgProject ('reports/emulator/run-' + [guid]::NewGuid().ToString('N'))
$sgStorage = Join-Path $sgRun 'storage'
$sgFolder = if ($Steam) { 'shotgunk2vita_steam' } else { 'shotgunk2vita' }
$sgData = Join-Path $sgStorage ('ux0/data/' + $sgFolder)
New-Item -ItemType Directory -Path (Split-Path $sgData -Parent) -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $ExistingStorage 'vs0') -Destination $sgStorage -Recurse
Copy-Item -LiteralPath $DataRoot -Destination $sgData -Recurse
if ($ReplayInput) {
    $sgReplay = Join-Path $sgData 'replay'
    if (Test-Path -LiteralPath $sgReplay) {
        # Preserve the copied recording in this isolated run.
        Move-Item -LiteralPath $sgReplay -Destination ($sgReplay + '-original')
    }
    New-Item -ItemType Directory -Path $sgReplay | Out-Null
    Copy-Item -LiteralPath $ReplayInput -Destination (Join-Path $sgReplay 'input.sgr')
    Copy-Item -LiteralPath $ReplaySnapshot -Destination (Join-Path $sgReplay 'snapshot') -Recurse
    $sgCfg = Join-Path $sgData 'performance.cfg'
    $sgText = if (Test-Path -LiteralPath $sgCfg) { Get-Content -LiteralPath $sgCfg -Raw } else { '' }
    $sgText = $sgText -replace '(?m)^input_(record|replay)=.*\r?\n?', ''
    [IO.File]::WriteAllText($sgCfg, $sgText + "`ninput_record=0`ninput_replay=1`n")
}
$sgConfig = Get-Content -LiteralPath $ExistingConfig -Raw
$sgPref = "pref-path: '" + $sgStorage.Replace("'","''") + "'"
if ($sgConfig -match '(?m)^pref-path:') { $sgConfig = $sgConfig -replace '(?m)^pref-path:.*$', $sgPref }
else { $sgConfig += "`n" + $sgPref + "`n" }
$sgConfig = $sgConfig -replace '(?m)^discord-rich-presence:.*$', 'discord-rich-presence: false'
$sgConfigFile = Join-Path $sgRun 'config.yml'
[IO.File]::WriteAllText($sgConfigFile, $sgConfig)
@{ vpk=(Resolve-Path -LiteralPath $Vpk).Path; sha256=(Get-FileHash -LiteralPath $Vpk -Algorithm SHA256).Hash; platform='Vita3K; hardware pending'; steam=[bool]$Steam; replay=[bool]$ReplayInput } | ConvertTo-Json | Set-Content (Join-Path $sgRun 'provenance.json')
$sgWindow = if ($Interactive) { 'Normal' } else { 'Hidden' }
Start-Process -FilePath $Emulator -WorkingDirectory $sgRun -WindowStyle $sgWindow -ArgumentList @('--frontend','sdl','--load-config','--keep-config','--config-location',('"'+$sgConfigFile+'"'),'--log-level','2',('"'+(Resolve-Path -LiteralPath $Vpk).Path+'"')) -PassThru
Write-Host "Isolated evidence: $sgRun"
