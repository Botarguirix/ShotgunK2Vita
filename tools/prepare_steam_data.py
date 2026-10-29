"""Prepare private Steam SGR assets for the experimental C/Lua runtime.

Prepara datos privados desde data.sgr, sin APK ni acceso a los saves existentes.
Requires Pillow and FFmpeg; extraction itself uses Python's standard library.
"""
import argparse
import hashlib
import json
import shutil
import struct
import subprocess
import tempfile
from pathlib import Path
from PIL import Image
from extract_sgr import unpack, extract, safe_relative


def prepare_images(root):
    with Image.open(root/'assets/gfx/gfx.png') as ref:
        rgb=ref.getpalette()
    if not rgb:
        raise ValueError('Reference image has no indexed palette')
    colors=[tuple(rgb[i:i+3]) for i in range(0,len(rgb),3)]
    palette=bytearray()
    for r,g,b in colors:palette.extend((r,g,b,255))
    palette.extend(bytes(1024-len(palette)))
    result=[]
    for path in sorted(root.rglob('*.png')):
        with Image.open(path) as im:
            w,h=im.size
            if im.mode=='P':pixels=im.tobytes()
            else:
                cache={};values=bytearray()
                for c in im.convert('RGBA').getdata():
                    if c[3]<128:idx=0
                    else:
                        key=c[:3]
                        if key not in cache:cache[key]=min(range(len(colors)),key=lambda i:sum((colors[i][k]-key[k])**2 for k in range(3)))
                        idx=cache[key]
                    values.append(idx)
                pixels=bytes(values)
            target=path.with_suffix('.sgimg')
            target.write_bytes(b'SGI1'+struct.pack('<II',w,h)+palette+pixels)
            result.append(path.relative_to(root).as_posix())
    return result


def prepare(source,output,ffmpeg):
    if output.exists():raise FileExistsError('Output exists; choose a new directory / La salida ya existe')
    if not ffmpeg or not Path(ffmpeg).is_file():raise ValueError('An explicit FFmpeg executable is required')
    output.parent.mkdir(parents=True,exist_ok=True)
    if not 0<source.stat().st_size<=512*1024*1024:raise ValueError('SGR input exceeds size limit')
    adapter=Path(__file__).resolve().parents[1]/'src/steam_compat.lua'
    if not adapter.is_file():raise FileNotFoundError('Missing Steam compatibility bridge')
    blob=source.read_bytes()
    digest=hashlib.sha256(blob).hexdigest()
    with tempfile.TemporaryDirectory(prefix='sg-steam-',dir=output.parent) as temp:
        stage=Path(temp);raw=unpack(blob,512*1024*1024)
        manifest=extract(raw,stage/'unpacked',{'source_name':source.name,'sha256':digest,'input_kind':'data.sgr'})
        game=stage/'game';game.mkdir();seen={};files=[]
        for entry in manifest['entries']:
            if entry['flag']:continue
            name=entry['name']
            while name.startswith('../'):name=name[3:]
            relative=safe_relative(name);key=relative.as_posix().casefold()
            if key in seen:
                if seen[key]!=entry['sha256']:raise ValueError('Conflicting duplicate resource: '+name)
                continue
            seen[key]=entry['sha256'];target=game/relative;target.parent.mkdir(parents=True,exist_ok=True)
            shutil.copy2(stage/'unpacked'/entry['output'],target);files.append(relative.as_posix())
        if not (game/'code.lua').is_file():raise ValueError('No code.lua in input')
        images=prepare_images(game)
        music={}
        for path in sorted(game.rglob('*.ogg')):
            target=path.with_suffix('.mp3')
            subprocess.run([str(ffmpeg),'-nostdin','-v','error','-i',str(path),'-codec:a','libmp3lame','-b:a','160k','-ar','44100','-ac','2',str(target)],check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
            music[path.relative_to(game).as_posix()]=target.relative_to(game).as_posix()
        (game/'save').mkdir();(game/'mods').mkdir()
        quote=lambda s:json.dumps(s,ensure_ascii=False)
        lua='return {files={'+','.join(quote(s) for s in sorted(files))+'},music={'
        lua+=','.join('['+quote(k)+']='+quote(v) for k,v in sorted(music.items()))+'}}\n'
        (game/'sg_steam_profile.lua').write_text(lua,encoding='utf-8')
        shutil.copy2(adapter,game/'sg_steam_compat.lua')
        profile={'source':'Steam data.sgr','source_sha256':digest,'metadata':manifest['metadata'],
                 'resources':len(files),'sgimg_images':len(images),'converted_music':len(music),
                 'apk_used':False,'original_lua_modified':False,'compatibility':'experimental; see branch report',
                 'save_format':'plain diagnostic only; not Steam encoded saves',
                 'languages_from_sgr':['safe_english'],
                 'direct_sgr_runtime_loading':False}
        profile['original_lua_sha256']={f:hashlib.sha256((game/f).read_bytes()).hexdigest() for f in files if f.endswith('.lua')}
        (game/'steam-prepared.json').write_text(json.dumps(profile,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
        shutil.move(str(game),str(output))
    return profile


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('sgr',type=Path)
    p.add_argument('output',type=Path)
    p.add_argument('--ffmpeg',required=True,type=Path)
    a=p.parse_args()
    print(json.dumps(prepare(a.sgr,a.output,a.ffmpeg),indent=2,ensure_ascii=False))


if __name__=='__main__':main()
