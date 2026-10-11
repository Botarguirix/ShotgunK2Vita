"""Package a locally built VPK and private prepared data for VitaShell."""
import hashlib
import json
import shutil
import zipfile
import argparse
import re
from pathlib import Path
from validate_vpk import validate

root = Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--vpk-only',action='store_true',help='Update the executable without rebuilding unchanged game data')
args=parser.parse_args()
iteration = re.search(r'VERSION "(\d{2}\.\d{2})"', (root/'CMakeLists.txt').read_text()).group(1)
source_vpk = root/'build-iterations'/iteration/'ShotgunK2Vita-performance.vpk'
validation = validate(source_vpk, require_art=(root/'private/livearea').is_dir())
if validation['sfo'].get('APP_VER') != iteration:
    raise ValueError('SFO version differs from source iteration')
out = root / 'artifacts/iterations' / iteration
# Never overwrite a previous delivery, even when packaging is retried.
out.mkdir(parents=True, exist_ok=False)
vpk = out / 'ShotgunK2Vita-performance.vpk'
shutil.copy2(source_vpk, vpk)
(out/'validation.json').write_text(json.dumps(validation,indent=2),encoding='utf-8')
snapshot = out/'source'
snapshot.mkdir()
for name in ('src', 'scripts', 'tools', 'docs', 'tests', '.github'):
    shutil.copytree(root/name, snapshot/name, ignore=shutil.ignore_patterns('__pycache__', '*_once.py'))
for name in ('CMakeLists.txt', 'README.md', '.gitignore', '.gitattributes'):
    shutil.copy2(root/name, snapshot/name)
third_party = snapshot/'third_party'
third_party.mkdir()
shutil.copytree(root/'third_party/lua-5.1.5/src', third_party/'lua-5.1.5/src')
for name in ('stb_truetype.h', 'dr_mp3.h'):
    shutil.copy2(root/'third_party'/name, third_party/name)
debug = out/'debug'
debug.mkdir()
shutil.copy2(source_vpk.parent/'ShotgunK2Vita', debug/'ShotgunK2Vita.elf')
shutil.copy2(source_vpk.parent/'ShotgunK2Vita.map', debug/'ShotgunK2Vita.map')
for name in ('performance.cfg', 'ShotgunK2Vita-PerformanceTool.zip'):
    if (root/'artifacts'/name).is_file():
        shutil.copy2(root/'artifacts'/name, out/name)
diagnostic_tool=out/'ShotgunK2Vita-DiagnosticsTool.zip'
with zipfile.ZipFile(diagnostic_tool,'w',zipfile.ZIP_DEFLATED) as z:
    z.write(root/'tools/collect_diagnostics.py','collect_diagnostics.py')
    z.write(root/'docs/CRASH_LOGS.md','README.md')
with zipfile.ZipFile(vpk) as z:
    assert z.testzip() is None
    assert {'eboot.bin', 'sce_sys/param.sfo'} <= set(z.namelist())
if args.vpk_only:
    manifest={p.relative_to(out).as_posix():{'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()}
              for p in (vpk,debug/'ShotgunK2Vita.elf',debug/'ShotgunK2Vita.map',diagnostic_tool)}
    (out/'SHA256-performance.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
    print(json.dumps(manifest,indent=2))
    raise SystemExit(0)
data = out / 'shotgunk2vita-data.zip'
source = root / 'private/game'
assert (source / 'code.lua').is_file()
with zipfile.ZipFile(data, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
    for p in sorted(source.rglob('*')):
        rel = p.relative_to(source)
        if not p.is_file() or rel.parts[0] in {'save', 'mods', 'logs'} or p.name.endswith(('.log', '.csv', '.cfg', '.psp2dmp')):
            continue
        z.write(p, 'shotgunk2vita/' + rel.as_posix())
    z.writestr('shotgunk2vita/save/', '')
    z.writestr('shotgunk2vita/save/mods/', '')
with zipfile.ZipFile(data) as z:
    assert z.testzip() is None
    assert 'shotgunk2vita/code.lua' in z.namelist()
    assert not any(n.endswith(('.so', '.apk', '.sav')) for n in z.namelist())
manifest = {p.name: {'bytes': p.stat().st_size, 'sha256': hashlib.sha256(p.read_bytes()).hexdigest()} for p in (vpk, data)}
(out / 'SHA256.json').write_text(json.dumps(manifest, indent=2), encoding='utf-8')
print(json.dumps(manifest, indent=2))
