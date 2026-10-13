"""Repeated, isolated desktop measurements. No inferred PS Vita framerate."""
import argparse
import hashlib
import html
import json
import platform
import re
import shutil
import statistics
import subprocess
import tempfile
from pathlib import Path
from perf_tool import read_samples, summary


def compare(config, groups, old):
    if old['config'] != config:
        raise ValueError('Baseline configuration/platform differs; do not compare these timings')
    comparisons = []
    for group in groups:
        prev = next(g for g in old['groups'] if (g['scene'], g['workers']) == (group['scene'], group['workers']))
        same = prev['fingerprint'] == group['fingerprint']
        gain = 100*(prev['median_mean_ms']-group['median_mean_ms'])/prev['median_mean_ms']
        noise = max(prev['max_mean_ms']-prev['min_mean_ms'], group['max_mean_ms']-group['min_mean_ms'])
        delta = abs(prev['median_mean_ms']-group['median_mean_ms'])
        verdict = 'different sequence: investigate' if not same else ('inconclusive' if abs(gain)<5 or delta<=noise else ('candidate improvement' if gain>0 else 'regression'))
        p95_gain = 100*(prev['median_p95_ms']-group['median_p95_ms'])/prev['median_p95_ms']
        if same and verdict == 'candidate improvement' and p95_gain < -5:
            verdict = 'mean improves but P95 regresses: investigate'
        comparisons.append({'scene': group['scene'], 'workers': group['workers'], 'mean_gain_percent': gain, 'p95_gain_percent': p95_gain, 'verdict': verdict})
    return comparisons


def run(args):
    root = Path(__file__).resolve().parent.parent
    version = re.search(r'set\(SG_BUILD_VERSION "([^"]+)"\)', (root/'CMakeLists.txt').read_text()).group(1)
    outbase = root/'reports'/'campaigns'/version
    outbase.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=outbase))
    binary = args.binary.resolve()
    config = {'platform': platform.platform(), 'frames': args.frames, 'skip': args.skip,
              'repeats': args.repeats, 'workers': args.workers, 'scenes': ['menu', 'board', 'actions'],
              'seed': 1234, 'viewport': '960x544', 'measurement': 'host Release wall time; not Vita FPS'}
    provenance = {'version': version, 'binary_sha256': hashlib.sha256(binary.read_bytes()).hexdigest(), 'config': config}
    try:
        provenance['commit'] = subprocess.check_output(['git', '-c', f'safe.directory={root}', 'rev-parse', 'HEAD'], cwd=root, text=True).strip()
        provenance['working_tree'] = subprocess.check_output(['git', '-c', f'safe.directory={root}', 'status', '--short'], cwd=root, text=True)
    except (OSError, subprocess.CalledProcessError):
        provenance['commit'] = 'unavailable'
    (out/'provenance.json').write_text(json.dumps(provenance, indent=2))
    (out/'status.json').write_text(json.dumps({'status': 'RUNNING', 'hardware': 'pending'}))
    print(f'Evidence: {out}', flush=True)
    records = []
    # On WSL, repeatedly copying game assets over /mnt/c is very expensive.
    # Copy once to a local temp cache; each case still has its own writable copy.
    scratch = tempfile.TemporaryDirectory(prefix='shotgunk2vita-campaign-')
    cache = Path(scratch.name)/'data'
    shutil.copytree(args.data, cache)
    # Rotate worker order to reduce a consistent first/last thermal bias.
    for repeat in range(args.repeats):
        order = args.workers[repeat % len(args.workers):] + args.workers[:repeat % len(args.workers)]
        for scene in config['scenes']:
            for workers in order:
                case = out/f'{scene}-w{workers}-pass{repeat+1}'
                case.mkdir()
                fixture = Path(scratch.name)/case.name
                shutil.copytree(cache, fixture)
                with (case/'runtime.log').open('w') as log:
                    try:
                        result = subprocess.run([str(binary), str(fixture), str(case/'performance.csv'), scene, str(args.frames), str(workers)], stdout=log, stderr=subprocess.STDOUT, timeout=180)
                    except subprocess.TimeoutExpired:
                        shutil.copytree(fixture, case/'failed-fixture')
                        (out/'status.json').write_text(json.dumps({'status': 'FAIL timeout', 'case': str(case)}))
                        raise
                if result.returncode:
                    shutil.copytree(fixture, case/'failed-fixture')
                    (out/'status.json').write_text(json.dumps({'status': 'FAIL', 'case': str(case)}))
                    raise RuntimeError(f'Failed case preserved: {case}')
                match = re.search(r'SCENE_RESULT scene=\w+ success=1 frames=(\d+) workers=(\d+) fingerprint=([0-9a-f]+)', (case/'runtime.log').read_text())
                if not match or int(match[1]) != args.frames or int(match[2]) != workers:
                    shutil.copytree(fixture, case/'failed-fixture')
                    (out/'status.json').write_text(json.dumps({'status': 'FAIL incomplete case', 'case': str(case)}))
                    raise RuntimeError(f'Incomplete case: {case}')
                stats = summary(read_samples(case/'performance.csv', args.skip))
                records.append({'scene': scene, 'workers': workers, 'pass': repeat+1, 'fingerprint': match[3], 'stats': stats})
                print(f'{scene} workers={workers} pass={repeat+1}: mean={stats["frame_mean_ms"]:.3f}ms p95={stats["frame_p95_ms"]:.3f}ms', flush=True)
                # Remove only our known temporary fixture, within this unique case.
                if fixture.resolve().parent != Path(scratch.name).resolve():
                    raise RuntimeError('Fixture cleanup escaped case')
                shutil.rmtree(fixture)
    scratch.cleanup()
    groups = []
    for scene in config['scenes']:
        hashes = {r['fingerprint'] for r in records if r['scene'] == scene}
        if len(hashes) != 1:
            (out/'status.json').write_text(json.dumps({'status': 'FAIL fingerprints differ', 'scene': scene}))
            raise RuntimeError(f'Non-equivalent sequence in {scene}; evidence preserved, timing not accepted')
        for workers in args.workers:
            cases = [r['stats'] for r in records if r['scene'] == scene and r['workers'] == workers]
            means = [s['frame_mean_ms'] for s in cases]
            groups.append({'scene': scene, 'workers': workers, 'fingerprint': next(iter(hashes)),
                           'median_mean_ms': statistics.median(means), 'min_mean_ms': min(means),
                           'max_mean_ms': max(means), 'median_p95_ms': statistics.median(s['frame_p95_ms'] for s in cases),
                           'stages_ms': {k: statistics.median(s['stages_ms'][k] for s in cases) for k in cases[0]['stages_ms']}})
            groups[-1]['dominant_stage'] = max(groups[-1]['stages_ms'], key=groups[-1]['stages_ms'].get)
    comparisons = []
    if args.baseline:
        old = json.loads(args.baseline.read_text())
        comparisons = compare(config, groups, old)
    report = {'config': config, 'provenance': provenance, 'groups': groups, 'comparisons': comparisons, 'runs': records, 'status': 'PASS host campaign; hardware pending'}
    (out/'summary.json').write_text(json.dumps(report, indent=2))
    (out/'status.json').write_text(json.dumps({'status': report['status']}))
    table = ''.join(f'<tr><td>{g["scene"]}</td><td>{g["workers"]}</td><td>{g["median_mean_ms"]:.3f}</td><td>{g["median_p95_ms"]:.3f}</td><td>{g["min_mean_ms"]:.3f}–{g["max_mean_ms"]:.3f}</td></tr>' for g in groups)
    (out/'report.html').write_text('<meta charset="utf-8"><title>ShotgunK2Vita campaign</title><h1>Pruebas PC; hardware pendiente</h1><p>Sin límite de FPS ni VSync. No predice FPS de Vita. Huellas de imagen equivalentes entre pases.</p><table border="1"><tr><th>Escena</th><th>Workers CPU</th><th>Mediana media ms</th><th>Mediana P95 ms</th><th>Rango medias ms</th></tr>'+table+'</table><pre>'+html.escape(json.dumps(comparisons, indent=2))+'</pre>', encoding='utf-8')
    print(report['status'], out/'report.html', flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--data', type=Path, default=Path('private/game'))
    parser.add_argument('--repeats', type=int, default=3)
    parser.add_argument('--frames', type=int, default=1200)
    parser.add_argument('--skip', type=int, default=120)
    parser.add_argument('--workers', type=int, nargs='+', default=[1, 3])
    parser.add_argument('--baseline', type=Path)
    args = parser.parse_args()
    if args.repeats<3 or args.frames<=args.skip or args.skip<0 or not args.workers or len(set(args.workers))!=len(args.workers) or any(w not in (1,2,3) for w in args.workers):
        parser.error('Need >=3 passes, frames > skip >=0, unique workers in 1..3')
    run(args)
