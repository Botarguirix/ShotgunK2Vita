"""Bundle locally copied Vita logs and optional crash dumps; never uploads files."""
import argparse
import json
import zipfile
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('logs', type=Path, help='Local copy of ux0:data/shotgunk2vita/logs/')
parser.add_argument('output', type=Path, help='New ZIP file; refuses overwrite')
parser.add_argument('--dump', type=Path, action='append', default=[], help='Optional .psp2dmp from ux0:data/')
args = parser.parse_args()
source = args.logs.resolve()
if not source.is_dir():
    parser.error('Log folder does not exist')
files = sorted(p for p in source.rglob('*') if p.is_file() and p.suffix.lower() in {'.log', '.csv', '.txt'})
if not files:
    parser.error('No log files found')
for dump in args.dump:
    if not dump.is_file() or dump.suffix.lower() != '.psp2dmp':
        parser.error('Each dump must be an existing .psp2dmp file')
summary = {}
for path in files:
    if path.name == 'crash-trace.log':
        text = path.read_text(encoding='utf-8', errors='replace')
        checkpoints = [line for line in text.splitlines() if line.startswith('checkpoint_us=')]
        endings = [line for line in text.splitlines() if line.startswith('session_end=')]
        summary[path.parent.name] = {'last_checkpoint': checkpoints[-1] if checkpoints else None,
                                    'session_end': endings[-1] if endings else 'missing: crash, forced exit or incomplete copy'}
with zipfile.ZipFile(args.output, 'x', zipfile.ZIP_DEFLATED) as archive:
    for path in files:
        archive.write(path, 'logs/'+path.relative_to(source).as_posix())
    for index, dump in enumerate(args.dump):
        archive.write(dump, f'dumps/{index}-{dump.name}')
    archive.writestr('summary.json', json.dumps(summary, indent=2))
print(f'Created {args.output}: {len(files)} log files, {len(args.dump)} dumps, {len(summary)} sessions')
