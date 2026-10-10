"""Package a locally built VPK and private prepared data for VitaShell."""
import hashlib
import json
import shutil
import zipfile
from pathlib import Path

root = Path(__file__).resolve().parents[1]
out = root / 'artifacts'
out.mkdir(exist_ok=True)
vpk = out / 'ShotgunK2Vita-bootstrap.vpk'
shutil.copy2(root / 'build' / vpk.name, vpk)
with zipfile.ZipFile(vpk) as z:
    assert z.testzip() is None
    assert {'eboot.bin', 'sce_sys/param.sfo'} <= set(z.namelist())
data = out / 'shotgunk2vita-data.zip'
source = root / 'private/game'
assert (source / 'code.lua').is_file()
with zipfile.ZipFile(data, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
    for p in sorted(source.rglob('*')):
        rel = p.relative_to(source)
        if not p.is_file() or rel.parts[0] in {'save', 'mods'} or p.name == 'runtime.log':
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
