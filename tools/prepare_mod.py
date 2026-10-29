"""Prepare a private mod for APK/1.5 or Steam game data (ES/EN).

--game 1.5 selects script1.5.lua when available (legacy default).
--game steam keeps Final Checkmate script.lua unchanged.
Existing installs require --replace and are backed up outside mods/.
"""
import argparse
import hashlib
import json
import re
import shutil
import struct
import tempfile
import uuid
from pathlib import Path
from PIL import Image


def validate_name(name):
    if not name or name in {'.','..'} or any(c in name for c in '/\\:\x00<>"|?*') or name.endswith((' ','.')) or any(ord(c)<32 for c in name):
        raise ValueError('Unsafe mod folder name / Nombre de mod inseguro')
    if re.fullmatch(r'(?i:con|prn|aux|nul|com[1-9]|lpt[1-9])(?:\..*)?',name):
        raise ValueError('Reserved folder name')
    return name


def convert_images(target,palette_path):
    with Image.open(palette_path) as ref:rgb=ref.getpalette()
    if not rgb:raise ValueError('Game palette must be indexed')
    colors=[tuple(rgb[i:i+3]) for i in range(0,len(rgb),3)]
    pal=bytearray()
    for r,g,b in colors:pal.extend((r,g,b,255))
    pal.extend(bytes(1024-len(pal)))
    converted=0
    for source in target.rglob('*.png'):
        with Image.open(source) as im:
            size=im.size
            if im.mode=='P':pixels=im.tobytes()
            else:
                cache={};values=bytearray()
                for c in im.convert('RGBA').getdata():
                    if c[3]<128:index=0
                    else:
                        key=c[:3]
                        if key not in cache:cache[key]=min(range(len(colors)),key=lambda i:sum((colors[i][k]-key[k])**2 for k in range(3)))
                        index=cache[key]
                    values.append(index)
                pixels=bytes(values)
            source.with_suffix('.sgimg').write_bytes(b'SGI1'+struct.pack('<II',*size)+pal+pixels)
            converted+=1
    return converted


def install_mod(mod,data,game='1.5',activate=False,replace=False):
    mod=Path(mod).resolve();data=Path(data).resolve()
    info=(mod/'info.lua').read_text(encoding='utf-8-sig',errors='strict')
    match=re.search(r'^\s*name\s*=\s*([\'"])(.*?)\1',info,re.M)
    if not match:raise ValueError('info.lua has no literal name field')
    name=validate_name(match.group(2))
    if game not in {'1.5','steam'}:raise ValueError('Unknown game profile')
    selected='script1.5.lua' if game=='1.5' and (mod/'script1.5.lua').is_file() else 'script.lua'
    if not (mod/selected).is_file():raise ValueError('Missing script for '+game+': '+selected)
    if not (data/'assets/gfx/gfx.png').is_file():raise ValueError('Prepared game palette missing')
    mods=data/'mods';mods.mkdir(parents=True,exist_ok=True)
    target=mods/name
    if target.is_symlink() or target.resolve().parent!=mods.resolve():raise ValueError('Unsafe destination')
    if target.exists() and not replace:raise FileExistsError('Mod exists; use --replace to back it up and replace it')
    # Symlinks must not pull arbitrary files into an installed mod.
    if any(p.is_symlink() for p in mod.rglob('*')):raise ValueError('Symlink entries are not supported')
    with tempfile.TemporaryDirectory(prefix='.sgmod-',dir=data) as temp:
        stage_root=Path(temp).resolve()
        if not stage_root.is_relative_to(data):raise ValueError('Unsafe temporary destination')
        stage=stage_root/name
        shutil.copytree(mod,stage,ignore=shutil.ignore_patterns('.git','__pycache__','save','logs'))
        if selected=='script1.5.lua':
            if (stage/'script.lua').exists():shutil.copy2(stage/'script.lua',stage/'script_final_checkmate.lua')
            shutil.copy2(stage/selected,stage/'script.lua')
        converted=convert_images(stage,data/'assets/gfx/gfx.png')
        digest=hashlib.sha256((stage/'script.lua').read_bytes()).hexdigest()
        report={'name':name,'game':game,'selected_script':selected,'installed_script_sha256':digest,'images':converted,'active':activate}
        (stage/'prepare-mod.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
        if target.exists():
            backups=data/'mod-backups';backups.mkdir(exist_ok=True)
            backup=backups/(name+'-'+uuid.uuid4().hex)
            target.rename(backup);report['backup']=str(backup)
        stage.rename(target)
    if activate:
        modlist=mods/'modlist.lua';entries=[]
        if modlist.exists():
            entries=[(m.group(2),m.group(3)) for m in re.finditer(r'\{\s*([\'"])(.*?)\1\s*,\s*(true|false)\s*\}',modlist.read_text(encoding='utf-8-sig'))]
        entries=[(n,a) for n,a in entries if n!=name]+[(name,'true')]
        modlist.write_text('return {\n'+''.join('  { '+json.dumps(n,ensure_ascii=False)+', '+a+' },\n' for n,a in entries)+'}\n',encoding='utf-8')
    return report


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('mod',type=Path)
    p.add_argument('data',type=Path)
    p.add_argument('--game',choices=['1.5','steam'],default='1.5',help='Script profile / Perfil de scripts')
    p.add_argument('--activate',action='store_true')
    p.add_argument('--replace',action='store_true',help='Back up and replace an installed mod / Respaldar y reemplazar')
    a=p.parse_args()
    print(json.dumps(install_mod(a.mod,a.data,a.game,a.activate,a.replace),indent=2,ensure_ascii=False))


if __name__=='__main__':main()
