"""Install a Shotgun King mod folder into prepared Vita data.

Copies the mod into <data>/mods/<name from info.lua> (the folder name the game
expects), prefers its script1.5.lua for the 1.5 game data, and writes a
.sgimg next to every PNG using the game palette exactly like
tools/prepare_data.py. Optionally marks it active in mods/modlist.lua.
Copy the resulting mods/ folder to ux0:data/shotgunk2vita/mods/.
"""
import argparse
import re
import shutil
import struct
from pathlib import Path
from PIL import Image

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('mod', type=Path, help='mod folder containing info.lua')
parser.add_argument('data', type=Path, help='prepared data root (has assets/gfx/gfx.png)')
parser.add_argument('--activate', action='store_true', help='add or enable it in mods/modlist.lua')
args = parser.parse_args()

info = (args.mod / 'info.lua').read_text(encoding='utf-8', errors='replace')
match = re.search(r'^\s*name\s*=\s*"([^"]+)"', info, re.M)
if not match:
    raise SystemExit('info.lua has no name="..." field')
name = match.group(1)
if '/' in name or '\\' in name or '..' in name:
    raise SystemExit('unsafe mod name: ' + name)

mods = args.data.resolve() / 'mods'
target = mods / name
if target.exists():
    shutil.rmtree(target)
shutil.copytree(args.mod, target)

# The prepared APK data is game version 1.5, whose loader always runs
# script.lua. Mods that support 1.5 ship script1.5.lua next to the Final
# Checkmate script.lua (that version swapped argument orders such as
# newsrf(file,name) -> newsrf(name,file)), so install the 1.5 one.
legacy = target / 'script1.5.lua'
if legacy.exists():
    if (target / 'script.lua').exists():
        (target / 'script.lua').replace(target / 'script_final_checkmate.lua')
    shutil.copyfile(legacy, target / 'script.lua')
    print('Using script1.5.lua for game version 1.5')
elif (target / 'script.lua').exists():
    print('WARNING: no script1.5.lua; this mod targets Shotgun King: The Final Checkmate and may not run on 1.5 data')

palette_image = Image.open(args.data / 'assets/gfx/gfx.png')
rgb = palette_image.getpalette()
colors = [tuple(rgb[i:i + 3]) for i in range(0, len(rgb), 3)]
converted = 0
for source in target.rglob('*.png'):
    image = Image.open(source)
    if image.mode == 'P':
        pixels = image.tobytes()
    else:
        cache = {}
        values = bytearray()
        for color in image.convert('RGBA').getdata():
            if color[3] < 128:
                index = 0
            else:
                key = color[:3]
                if key not in cache:
                    cache[key] = min(range(len(colors)), key=lambda i: sum((colors[i][c] - key[c]) ** 2 for c in range(3)))
                index = cache[key]
            values.append(index)
        pixels = bytes(values)
    pal = bytearray()
    for r, g, b in colors:
        pal.extend((r, g, b, 255))
    pal.extend(bytes(1024 - len(pal)))
    source.with_suffix('.sgimg').write_bytes(b'SGI1' + struct.pack('<II', *image.size) + pal + pixels)
    converted += 1

if args.activate:
    modlist = mods / 'modlist.lua'
    entries = []
    if modlist.exists():
        entries = re.findall(r"\{\s*'([^']*)'\s*,\s*(true|false)\s*\}", modlist.read_text(encoding='utf-8'))
    entries = [(n, a) for n, a in entries if n != name] + [(name, 'true')]
    modlist.write_text('return {\n' + ''.join("\t{ '%s', %s },\n" % e for e in entries) + '}', encoding='utf-8')

print('Installed mod', repr(name), 'images', converted, 'active' if args.activate else '')
