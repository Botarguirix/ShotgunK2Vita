param(
    [string]$Emulator = 'C:/Users/ikari/Downloads/Vita/windows-latest/Vita3K.exe',
    [string]$ExistingStorage = 'C:/Users/ikari/AppData/Roaming/Vita3K/Vita3K',
    [string]$ExistingConfig = 'C:/Users/ikari/Downloads/Vita/windows-latest/config.yml'
)
$ErrorActionPreference = 'Stop'
$sgProject = Split-Path $PSScriptRoot -Parent
$sgCmake = Get-Content (Join-Path $sgProject 'CMakeLists.txt') -Raw
$sgVersion = [regex]::Match($sgCmake, 'set\(SG_BUILD_VERSION "([^"]+)"\)').Groups[1].Value
$sgDelivery = Join-Path $sgProject "artifacts/iterations/$sgVersion"
$sgVpk = @(Get-ChildItem -LiteralPath $sgDelivery -Filter '*.vpk')
if ($sgVpk.Count -ne 1) { throw 'Expected exactly one VPK in the current delivery.' }
if (!(Test-Path -LiteralPath $Emulator)) { throw 'Vita3K executable missing.' }
$sgRun = Join-Path $sgProject ('reports/emulator/' + $sgVersion + '/run-' + [guid]::NewGuid().ToString('N'))
$sgStorage = Join-Path $sgRun 'storage'
New-Item -ItemType Directory -Path $sgStorage -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $ExistingStorage 'vs0') -Destination $sgStorage -Recurse
New-Item -ItemType Directory -Path (Join-Path $sgStorage 'ux0/data') -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $sgProject 'private/game') -Destination (Join-Path $sgStorage 'ux0/data/shotgunk2vita') -Recurse
$sgConfig = Get-Content -LiteralPath $ExistingConfig -Raw
$sgConfig = $sgConfig -replace '(?m)^pref-path:.*$', ('pref-path: ' + $sgStorage + '\')
$sgConfig = $sgConfig -replace '(?m)^discord-rich-presence:.*$', 'discord-rich-presence: false'
$sgConfigFile = Join-Path $sgRun 'config.yml'
[IO.File]::WriteAllText($sgConfigFile, $sgConfig)
$sgHash = (Get-FileHash -LiteralPath $sgVpk[0].FullName -Algorithm SHA256).Hash
@{ version=$sgVersion; vpk=$sgVpk[0].FullName; sha256=$sgHash; platform='Vita3K, not hardware'; started=(Get-Date).ToString('o') } | ConvertTo-Json | Set-Content (Join-Path $sgRun 'provenance.json')
Write-Host "Isolated evidence: $sgRun"
Write-Host 'Close other Vita3K sessions first: emulator logs may be shared beside its executable.'
# Interactive test window requested by the user. VPK path avoids installed-id
# validation against the emulator's usual profile before custom config is loaded.
Start-Process -FilePath $Emulator -WorkingDirectory $sgRun -ArgumentList @('--frontend','sdl','--load-config','--keep-config','--config-location',('"'+$sgConfigFile+'"'),'--log-level','2',('"'+$sgVpk[0].FullName+'"')) -PassThru
