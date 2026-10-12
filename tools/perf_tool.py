"""Offline performance reports and A/B configuration for ShotgunK2Vita."""
import argparse
import csv
import html
import json
import math
import statistics
import re
from pathlib import Path

PRESETS = {
    'balanced': {'renderer': 1, 'cpu_mhz': 444, 'workers': 3, 'copy_mode': 2, 'profile': 1},
    'single': {'renderer': 0, 'cpu_mhz': 444, 'workers': 1, 'copy_mode': 2, 'profile': 1},
    'copy-cpu': {'renderer': 0, 'cpu_mhz': 444, 'workers': 3, 'copy_mode': 0, 'profile': 1},
    'copy-dma': {'renderer': 0, 'cpu_mhz': 444, 'workers': 3, 'copy_mode': 1, 'profile': 1},
    'battery': {'renderer': 1, 'cpu_mhz': 333, 'workers': 2, 'copy_mode': 2, 'profile': 1},
}
STAGES = ('update_ms', 'draw_ms', 'present_ms', 'wait_ms')

def read_samples(path, skip):
    with path.open(encoding='utf-8-sig', newline='') as f:
        raw = list(csv.DictReader(f))
    rows = []
    stop_frame = None
    runtime = path.with_name('runtime.log')
    if runtime.is_file():
        stopped = re.search(r'native_frames=(\d+) running=0', runtime.read_text(encoding='utf-8', errors='replace'))
        if stopped: stop_frame = int(stopped.group(1))
    for r in raw:
        try:
            r = {k: float(v) for k, v in r.items()}
            if r.get('running', 1) == 0 or (stop_frame is not None and r.get('frame', 0) >= stop_frame):
                continue
            if not all(math.isfinite(r[k]) and r[k] >= 0 for k in (*STAGES, 'frame_ms')):
                continue
            if r['frame_ms'] > 0: rows.append(r)
        except (KeyError, TypeError, ValueError):
            continue
    rows = rows[skip:]
    if not rows: raise ValueError('No valid samples remain; reduce --skip-first')
    return rows

def entity_summary(rows):
    result={}
    for k in ('entities','entity_updates','entity_draws','palette_resets','palette_suppressed','sprite_calls','blit_pixels','shadow_calls','shadow_pixels','shadow_fallbacks'):
        if not any(k in r for r in rows): continue
        values=[r.get(k,0) for r in rows]; draw=[r['draw_ms'] for r in rows]
        corr=None
        if len(rows)>1 and statistics.pstdev(values)>0 and statistics.pstdev(draw)>0:
            mx,my=statistics.mean(values),statistics.mean(draw)
            corr=sum((a-mx)*(b-my) for a,b in zip(values,draw))/(len(rows)*statistics.pstdev(values)*statistics.pstdev(draw))
        result[k]={'mean':statistics.mean(values),'max':max(values),'draw_correlation':corr}
    return result

def summary(rows):
    frames = sorted(r['frame_ms'] for r in rows)
    return {'samples': len(rows), 'fps': 1000/statistics.mean(frames),
        'frame_mean_ms': statistics.mean(frames), 'frame_p95_ms': frames[math.ceil(.95*len(frames))-1],
        'stages_ms': {k: statistics.mean(r[k] for r in rows) for k in STAGES},
        'entity_metrics': entity_summary(rows),
        'renderers': sorted(set(int(r.get('renderer',0)) for r in rows)),
        'gpu_submit_mean_ms': statistics.mean(r.get('gpu_submit_ms',0) for r in rows),
        'copy_mean_ms': statistics.mean(r.get('copy_ms', 0) for r in rows),
        'input_counts': {k: sum(r.get(k,0) for r in rows) for k in ('input_calls','input_evaluations','input_parses')} if any('input_calls' in r for r in rows) else {},
        'copy_modes': sorted(set(int(r['copy_mode']) for r in rows if 'copy_mode' in r)),
        'copy_status_codes': sorted(set(int(r['copy_rc']) for r in rows if 'copy_rc' in r)),
        'glyph_misses': sum(r.get('glyph_misses', 0) for r in rows),
        'render_participants_ms': {k: statistics.mean(r[k] for r in rows) for k in ('render0_ms','render1_ms','render2_ms') if all(k in r for r in rows)},
        'workers': sorted(set(int(r.get('workers', 1)) for r in rows)),
        'cpu_mhz': sorted(set(int(r.get('cpu_mhz', 0)) for r in rows))}

def report(args):
    rows = read_samples(args.csv, args.skip_first)
    stats = summary(rows)
    baseline = summary(read_samples(args.baseline, args.skip_first)) if args.baseline else None
    name = html.escape(args.csv.name)
    table = ''.join(f'<tr><td>{html.escape(k)}</td><td>{v:.3f} ms</td></tr>' for k, v in stats['stages_ms'].items())
    table += f'<tr><td>Copia / subida de textura</td><td>{stats["copy_mean_ms"]:.3f} ms</td></tr>'
    table += '<tr><td>Renderer</td><td>'+str(stats['renderers'])+' (0 CPU, 1 GPU)</td></tr>'
    if 1 in stats['renderers']:
        table += f'<tr><td>Envío GXM (CPU)</td><td>{stats["gpu_submit_mean_ms"]:.3f} ms</td></tr><tr><td colspan="2">wait_ms incluye cola GPU y vblank; FPS se mide después de completar la presentación.</td></tr>'
    if stats['input_counts']:
        table += '<tr><td>Entrada: consultas / evaluaciones / parseos</td><td>'+ ' / '.join(str(int(stats['input_counts'][k])) for k in ('input_calls','input_evaluations','input_parses')) + '</td></tr>'
    if stats['copy_modes']:
        backends = ', '.join('DMA' if m else 'CPU' for m in stats['copy_modes'])
        table += f'<tr><td>Backend de copia</td><td>{backends}; rc={stats["copy_status_codes"]}</td></tr>'
    if stats['render_participants_ms']:
        table += '<tr><td colspan="2">Trabajo de filas por participante (en paralelo; no sumar a frame_ms)</td></tr>'
        table += ''.join(f'<tr><td>{k}</td><td>{v:.3f} ms</td></tr>' for k,v in stats['render_participants_ms'].items())
    for k,v in stats['entity_metrics'].items():
        corr='sin variación suficiente' if v['draw_correlation'] is None else f'{v["draw_correlation"]:.3f}'
        table += f'<tr><td>{html.escape(k)}</td><td>media {v["mean"]:.1f}; máximo {v["max"]:.0f}; correlación con draw_ms: {corr}</td></tr>'
    if 'entities' in stats['entity_metrics']:
        table += '<tr><td colspan="2">Las llamadas de dibujo incluyen callbacks que omiten entidades invisibles. Correlación no demuestra causalidad; comparar la misma escena.</td></tr>'
    dominant = max(STAGES[:3], key=lambda k: stats['stages_ms'][k])
    notes = {
        'update_ms': 'La lógica y los callbacks Lua concentran el mayor tiempo de trabajo. Más hilos de presentación no paralelizan ese estado Lua.',
        'draw_ms': 'El dibujo concentra el mayor tiempo de trabajo. Revisar letras nuevas, áreas dibujadas y llamadas de scripts antes de aumentar hilos.',
        'present_ms': 'La presentación concentra el mayor tiempo de trabajo. Comparar balanced y single en la misma pantalla para comprobar el beneficio de los hilos.',
    }
    comparison = '' if not baseline else f'<p>Referencia: {baseline["frame_mean_ms"]:.3f} ms por fotograma. Razón de tiempos: {baseline["frame_mean_ms"]/stats["frame_mean_ms"]:.2f}×. Comparar sólo recorridos y ajustes equivalentes.</p>'
    if stats['cpu_mhz'] == [0]:
        comparison += '<p><strong>Prueba de PC sin límite de fotogramas: los FPS derivados NO son FPS medidos en Vita.</strong></p>'
    # At most 800 bins; retain peaks as well as means so stalls remain visible.
    stride = max(1, math.ceil(len(rows)/800))
    chart = [[int(rows[i]['frame']), *[statistics.mean(r[k] for r in rows[i:i+stride]) for k in STAGES], max(r['frame_ms'] for r in rows[i:i+stride])] for i in range(0,len(rows),stride)]
    document = '''<!doctype html><html lang="es"><meta charset="utf-8"><title>ShotgunK2Vita · Rendimiento</title>
<style>body{background:#0e1221;color:#ffd0ad;font:16px system-ui;max-width:1100px;margin:36px auto;padding:0 24px}h1{font-size:32px}section{background:#22263d;border-radius:16px;padding:22px;margin:20px 0}table{border-collapse:collapse;width:100%}td{padding:8px;border-bottom:1px solid #4a4560}.big{font-size:28px}canvas{width:100%;height:340px}label{margin-right:18px}small{color:#bcb0ca}</style>
<h1>ShotgunK2Vita · Rendimiento</h1><p>FILE_NAME</p>
<section><p class="big">FPS_VALUE FPS · MEAN_VALUE ms por fotograma</p><p>P95: P95_VALUE ms · SAMPLES_VALUE muestras</p>COMPARE_VALUE</section>
<section><h2>Tiempo medio por etapa</h2><table>TABLE_VALUE</table><p>NOTE_VALUE</p><p>La espera incluye sincronización con la pantalla; incluye cola GPU en renderer=1. El tiempo total también incluye entrada, registros y otras tareas.</p></section>
<section><h2>Evolución de tiempos</h2><p id="checks"></p><canvas id="plot" width="1000" height="340"></canvas><small>Medias por intervalo y pico de tiempo total. Líneas discontinuas: 33,33 ms (30 FPS) y 16,67 ms (60 FPS).</small></section>
<section><p>Workers: WORKERS_VALUE · CPU: CPU_VALUE MHz · Letras rasterizadas: MISSES_VALUE</p><p>Probar balanced (GPU) y copy-cpu durante el mismo recorrido. Conservar la carpeta completa de cada sesión; las ejecuciones anteriores no se sobrescriben. Las muestras detenidas se excluyen si hay running o runtime.log contiguo.</p><p>Una medición de PC no predice FPS en Vita. El audio tiene su propio registro audio.log con bloques tardíos y errores de salida.</p></section>
<script>const data=CHART_DATA, names=['Lua/update','Dibujo','Presentación','Espera','Pico total'],colors=['#72b9ff','#ff7289','#9ce1ac','#bbb0dc','#ffd0ad'];
let enabled=names.map(()=>true);document.getElementById('checks').innerHTML=names.map((n,i)=>`<label style="color:${colors[i]}"><input type="checkbox" checked onchange="enabled[${i}]=this.checked;draw()">${n}</label>`).join('');
function draw(){const c=document.getElementById('plot'),g=c.getContext('2d'),w=c.width,h=c.height;g.clearRect(0,0,w,h);const limit=Math.max(40,...data.flatMap(r=>r.slice(1).filter((_,i)=>enabled[i])));g.font='14px system-ui';g.fillStyle='#bcb0ca';
for(let i=0;i<=4;i++){let y=20+i*(h-50)/4;g.strokeStyle='#44465c';g.beginPath();g.moveTo(60,y);g.lineTo(w-20,y);g.stroke();g.fillText((limit*(4-i)/4).toFixed(1)+' ms',2,y+5)}
g.setLineDash([5,5]);g.strokeStyle='#70758e';for(const budget of [33.333,16.667]){g.beginPath();const target=h-30-budget/limit*(h-50);g.moveTo(60,target);g.lineTo(w-20,target);g.stroke()}g.setLineDash([]);
names.forEach((_,j)=>{if(!enabled[j])return;g.strokeStyle=colors[j];g.beginPath();data.forEach((r,i)=>{let x=60+i/Math.max(1,data.length-1)*(w-80),y=h-30-r[j+1]/limit*(h-50);if(i)g.lineTo(x,y);else g.moveTo(x,y)});g.stroke()});g.fillText('Fotograma '+data[0][0],60,h-5);g.fillText('Fotograma '+data[data.length-1][0],w-170,h-5)}draw();</script></html>'''
    values = {'FILE_NAME': name, 'FPS_VALUE': f'{stats["fps"]:.1f}', 'MEAN_VALUE': f'{stats["frame_mean_ms"]:.3f}', 'P95_VALUE': f'{stats["frame_p95_ms"]:.3f}', 'SAMPLES_VALUE': str(stats['samples']), 'COMPARE_VALUE': comparison, 'TABLE_VALUE': table, 'NOTE_VALUE': notes[dominant], 'WORKERS_VALUE': str(stats['workers']), 'CPU_VALUE': str(stats['cpu_mhz']), 'MISSES_VALUE': str(int(stats['glyph_misses'])), 'CHART_DATA': json.dumps(chart)}
    for key, value in values.items(): document = document.replace(key, value)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(document, encoding='utf-8')
    args.output.with_suffix('.json').write_text(json.dumps({'current':stats,'baseline':baseline},indent=2),encoding='utf-8')
    print(json.dumps(stats, indent=2));print('Report:', args.output.resolve())

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    subs = parser.add_subparsers(dest='command', required=True)
    config = subs.add_parser('config')
    config.add_argument('--preset', choices=PRESETS, default='balanced')
    config.add_argument('--output', type=Path, default=Path('performance.cfg'))
    view = subs.add_parser('report')
    view.add_argument('csv', type=Path)
    view.add_argument('--baseline', type=Path)
    view.add_argument('--skip-first', type=int, default=0)
    view.add_argument('--output', type=Path, default=Path('performance-report.html'))
    args = parser.parse_args()
    if args.command == 'config':
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(''.join(f'{k}={v}\n' for k,v in PRESETS[args.preset].items()),encoding='ascii')
        print('Copy to ux0:data/shotgunk2vita/performance.cfg:', args.output.resolve())
    else: report(args)

if __name__ == '__main__': main()
