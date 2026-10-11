"""Audit known SUGAR dependencies and image costs locally without editing data."""
import argparse
import json
import re
import struct
import zlib
from pathlib import Path

# Names from the public SUGAR interface, including routes not yet exercised.
APIS = '''aspr asspr circ circfill tri trifill spr sspr sget sset pget pset
rect rectfill line cls color camera tcamera clip pal palt fillp sfillp
newsrf delsrf spritesheet sprgrid palette target srfsize newwin winspec
newfnt font fntspec strwidth bprint print fpslimit dt delta t time sugar_step
btn btnp btnr btnv controls defbtn ctrlr mouse rumble btnclr inpnum
file afile encfile ls mkdir isfile isfolder rm desktop_path syslang
newbnk delbnk bank bnksize bget bset expbnk savbnk band bor bxor bnot shl shr
lshr rotl rotr newsfx newmus delsfx delmus sfx mus music sfxvol musvol
sfxlen muslen chnlfx chnlprog nxtmusic shader shdrf shdri shdrsrf
lockaudio unlockaudio freeze sleep fb_ev quit ismobile steam psn live url hex'''.split()
PARTIAL = {'aspr':'uses the asspr rasterizer; visual fidelity still to verify',
           'asspr':'CPU rotation; fidelity and clipping need broader coverage',
           'circ':'bounded outline; SUGAR subpixel diameter fidelity pending',
           'bget':'SET option descriptors only; numeric legacy banks unsupported',
           'bset':'SET option descriptors only; numeric legacy banks unsupported',
           'encfile':'legacy encoded save import/export unsupported',
           'afile':'synchronous compatibility path, not background IO',
           'winspec':'currently ignored; presentation handles a fixed overlay',
           'fpslimit':'ignored; native display synchronization controls pacing',
           'shdrf':'ignored in shaderless mode',
           'rumble':'no device effect', 'steam':'no integration',
           'live':'no integration', 'psn':'no integration',
           'dt':'fixed 1/60 step rather than elapsed wall time',
           'delta':'fixed 1/60 step rather than elapsed wall time',
           'music':'MP3/WAV; incomplete crossfade/channel semantics',
           'mus':'MP3/WAV; incomplete crossfade/channel semantics',
           'sfx':'WAV PCM16; OGG unsupported'}


def code_only(text):
    # Mask comments/strings, including arbitrary Lua long-bracket delimiters.
    pattern=r'--\[(=*)\[.*?\]\1\]|\[(=*)\[.*?\]\2\]|--[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\''
    return re.sub(pattern, lambda m: '\n'*m.group(0).count('\n')+' ', text, flags=re.S)


def audit(data, source):
    if not (data/'code.lua').is_file(): raise ValueError('Prepared game root must contain code.lua')
    if not (source/'src').is_dir(): raise ValueError('Runtime source directory not found')
    native='\n'.join(p.read_text(encoding='utf-8') for p in (source/'src').glob('*.c'))
    registered=set(re.findall(r'(?:reg|alias)\(\s*(?:L\s*,\s*)?"([A-Za-z_][A-Za-z_0-9]*)"',native))
    uses={name:[] for name in APIS}
    scripts=set()
    for p in sorted(data.rglob('*.lua')):
        if p.relative_to(data).parts[0] in {'save','mods','logs'}: continue
        code=code_only(p.read_text(encoding='utf-8',errors='replace'))
        scripts.update(re.findall(r'\bfunction\s+([A-Za-z_]\w*)\s*\(',code))
        scripts.update(re.findall(r'(?m)^\s*([A-Za-z_]\w*)\s*=\s*function\b',code))
        for name in APIS:
            # Include captured function values such as bind(aspr,...).
            if re.search(r'\b'+name+r'\b',code): uses[name].append(p.relative_to(data).as_posix())
    entries=[]
    for name, files in uses.items():
        if not files: continue
        status='partial' if name in PARTIAL and name in registered else 'registered' if name in registered else 'script_defined' if name in scripts else 'missing'
        entries.append({'api':name,'status':status,'script_defined':name in scripts,'files':files,'limitation':PARTIAL.get(name,'')})
    images=[]
    for p in sorted(data.rglob('*.sgimg')):
        raw=p.read_bytes()
        if len(raw)<1036 or raw[:4]!=b'SGI1': continue
        w,h=struct.unpack_from('<II',raw,4)
        images.append({'path':p.relative_to(data).as_posix(),'width':w,'height':h,
                       'runtime_pixels_bytes':w*h,'sgimg_bytes':len(raw),
                       'deflate_storage_estimate_bytes':len(zlib.compress(raw,6))})
    return {'schema':1,'scope':'known API token inventory, not execution or full Lua scope analysis',
            'hardware_validation':'pending','apis':entries,'images':images,
            'pixel_memory_bytes':sum(i['runtime_pixels_bytes'] for i in images),
            'note':'SGI1 is already 8-bit indexed. Compression changes storage/load/decode, not pixels processed each frame. Do not resize sprite atlases: coordinates would change.'}


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('data',type=Path)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--source',type=Path,help='Runtime repository or bundled api-source folder')
    args=parser.parse_args()
    source=args.source or Path(__file__).resolve().parents[1]
    if not (source/'src').is_dir(): source=Path(__file__).resolve().parent/'api-source'
    result=audit(args.data,source)
    args.output.parent.mkdir(parents=True,exist_ok=True)
    with args.output.open('x',encoding='utf-8') as f: json.dump(result,f,indent=2,ensure_ascii=False)
    print(json.dumps({'missing':[a['api'] for a in result['apis'] if a['status']=='missing'],
                      'partial':[a['api'] for a in result['apis'] if a['status']=='partial'],
                      'images':len(result['images']),'pixel_memory_bytes':result['pixel_memory_bytes']},indent=2))

if __name__=='__main__': main()
