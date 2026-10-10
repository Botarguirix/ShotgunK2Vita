"""Prepare local game data from an APK; never execute its native libraries."""
import argparse
import json
import zipfile
import struct
from PIL import Image
from pathlib import Path, PurePosixPath

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('apk', type=Path)
parser.add_argument('output', type=Path)
args = parser.parse_args()
root = args.output.resolve()
root.mkdir(parents=True, exist_ok=True)
count = 0
with zipfile.ZipFile(args.apk) as archive:
    for info in archive.infolist():
        name = info.filename
        if not name.startswith('assets/') or info.is_dir(): continue
        relative = PurePosixPath(name[len('assets/'):])
        if '..' in relative.parts or relative.is_absolute(): raise ValueError('Unsafe archive path')
        if not (relative.suffix.lower() in {'.lua','.png','.wav','.mp3','.ogg','.txt','.ttf','.shader','.bnk'}): continue
        target = root.joinpath(*relative.parts).resolve()
        if not target.is_relative_to(root): raise ValueError('Unsafe destination')
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(archive.read(info))
        count += 1
(root/'save').mkdir(exist_ok=True)
palette_image = Image.open(root/'assets/gfx/gfx.png')
rgb = palette_image.getpalette()
colors = [tuple(rgb[i:i+3]) for i in range(0,len(rgb),3)]
for source in root.rglob('*.png'):
    image = Image.open(source)
    if image.mode == 'P':
        pixels = image.tobytes()
    else:
        rgba = image.convert('RGBA')
        cache = {}
        values = bytearray()
        for color in rgba.getdata():
            if color[3] < 128: index = 0
            else:
                key=color[:3]
                if key not in cache: cache[key]=min(range(len(colors)),key=lambda i:sum((colors[i][c]-key[c])**2 for c in range(3)))
                index=cache[key]
            values.append(index)
        pixels=bytes(values)
    pal=bytearray()
    for r,g,b in colors: pal.extend((r,g,b,255))
    pal.extend(bytes(1024-len(pal)))
    source.with_suffix('.sgimg').write_bytes(b'SGI1'+struct.pack('<II',*image.size)+pal+pixels)
(root/'prepared.json').write_text(json.dumps({'files':count,'native_libraries_included':False},indent=2))
print('Prepared',count,'files in',root)
