"""Write the delivery audit once, preserving existing iterations."""
import hashlib
import json
import shutil
import subprocess
from pathlib import Path


def finalize(root, out, iteration):
    root, out = Path(root), Path(out)
    if (out / 'delivery-audit.json').exists():
        raise FileExistsError(f'Delivery already audited: {out}')
    # Reports stay private, including test fixtures and original received logs.
    reports = root / 'reports/iterations' / iteration
    if reports.is_dir():
        shutil.copytree(reports, out / 'evidence')
    for name in (f'STATUS_ITERATION{iteration[-2:]}.md',
                 f'INFORME_ROADMAP_ITERACION{iteration[-2:]}.md'):
        p = root / 'docs' / name
        if p.is_file():
            shutil.copy2(p, out / name)
    provenance = {}
    for key, args in [('commit', ['rev-parse', 'HEAD']),
                      ('working_tree', ['status', '--porcelain', '--untracked-files=normal'])]:
        try:
            result = subprocess.run(['git', '-c', f'safe.directory={root.as_posix()}',
                                     '-C', str(root), *args], capture_output=True,
                                    text=True, check=True)
            provenance[key] = result.stdout.strip()
        except (OSError, subprocess.CalledProcessError):
            provenance[key] = 'unavailable'
    manifest = {}
    for p in sorted(out.rglob('*')):
        if p.is_file():
            digest = hashlib.sha256()
            with p.open('rb') as stream:
                for chunk in iter(lambda: stream.read(1024 * 1024), b''):
                    digest.update(chunk)
            manifest[p.relative_to(out).as_posix()] = {
                'bytes': p.stat().st_size, 'sha256': digest.hexdigest()}
    record = {'schema': 1, 'version': iteration, 'source': provenance,
              'hardware_validation': 'pending', 'files': manifest}
    with (out / 'delivery-audit.json').open('x', encoding='utf-8') as stream:
        json.dump(record, stream, indent=2, ensure_ascii=False)
    return record
