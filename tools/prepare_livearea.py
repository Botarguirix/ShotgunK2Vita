"""Extract RT_ICON from a Steam PE without running it; build private LiveArea art."""
import argparse
import io
import json
import struct
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('exe', type=Path)
parser.add_argument('game_data', type=Path)
parser.add_argument('output', type=Path)
args = parser.parse_args()
data = args.exe.read_bytes()
u16 = lambda p: struct.unpack_from('<H', data, p)[0]
u32 = lambda p: struct.unpack_from('<I', data, p)[0]
pe = u32(0x3c)
assert data[:2] == b'MZ' and data[pe:pe+4] == b'PE\0\0'
optional = pe+24
directory = optional+(112 if u16(optional) == 0x20b else 96)
sections = optional+u16(pe+20)

def offset(rva):
    for i in range(u16(pe+6)):
        p = sections+i*40
        if u32(p+12) <= rva < u32(p+12)+u32(p+16):
            return u32(p+20)+rva-u32(p+12)
    raise ValueError('Unmapped resource RVA')

base = offset(u32(directory+16))
resources = {}

def walk(relative, path=()):
    if len(path) > 4: raise ValueError('Invalid resource depth')
    p = base+relative
    for i in range(u16(p+12)+u16(p+14)):
        name, entry = struct.unpack_from('<II', data, p+16+i*8)
        key = name if not name & 0x80000000 else 'named'
        if entry & 0x80000000:
            walk(entry & 0x7fffffff, path+(key,))
        else:
            rva, size = struct.unpack_from('<II', data, base+entry)
            start = offset(rva)
            if start+size > len(data): raise ValueError('Truncated resource')
            resources[path+(key,)] = data[start:start+size]

walk(0)
groups = [v for k, v in resources.items() if k[0] == 14]
if not groups: raise ValueError('Executable has no RT_GROUP_ICON')
group = groups[0]
count = struct.unpack_from('<H', group, 4)[0]
entries, bodies = [], []
cursor = 6+16*count
for i in range(count):
    width, height, colors, reserved, planes, bits, size, icon_id = struct.unpack_from('<BBBBHHIH', group, 6+i*14)
    body = next(v for k, v in resources.items() if k[0] == 3 and k[1] == icon_id)
    entries.append(struct.pack('<BBBBHHII', width, height, colors, reserved, planes, bits, len(body), cursor))
    bodies.append(body)
    cursor += len(body)
ico = struct.pack('<HHH', 0, 1, count)+b''.join(entries)+b''.join(bodies)
out = args.output
contents = out/'livearea/contents'
contents.mkdir(parents=True, exist_ok=True)
(out/'steam-extracted.ico').write_bytes(ico)
original = Image.open(io.BytesIO(ico)).convert('RGBA')
icon = original.resize((128, 128), Image.Resampling.NEAREST)
icon.save(out/'icon0.png')

# The Steam executable supplies its icon; the APK supplies original title art.
logo_source = args.game_data/'assets/gfx/title.png'
atlas = Image.open(logo_source).convert('RGBA')
def transparent_black(im):
    alpha = Image.new('L', im.size)
    alpha.putdata([255 if pixel[:3] != (0,0,0) else 0 for pixel in im.getdata()])
    im.putalpha(alpha)
    return im
logo = transparent_black(atlas.crop((320,0,464,96)))
bg_source = logo_source
bg = atlas.crop((0,0,320,180)).convert('RGB').resize((840,500), Image.Resampling.NEAREST)
king = transparent_black(atlas.crop((0,180,160,352))).resize((320,344),Image.Resampling.NEAREST)
bg.paste(king,(24,156),king)
logo = logo.resize((432,288),Image.Resampling.NEAREST)
bg.paste(logo,(384,40),logo)
draw = ImageDraw.Draw(bg)
font = ImageFont.truetype(str(args.game_data/'assets/fonts/Terminus.ttf'),22)
draw.rectangle((0,464,840,500),fill=(14,18,33))
draw.text((420,481),'ShotgunK2Vita  |  Experimental',font=font,fill=(245,190,150),anchor='mm')
bg.convert('RGBA').save(contents/'bg0.png')
bg.resize((960, 544), Image.Resampling.NEAREST).convert('RGBA').save(out/'pic0.png')
startup = Image.new('RGBA', (280, 158), (14,18,33,255))
small = logo.copy(); small.thumbnail((250, 100), Image.Resampling.NEAREST)
startup.paste(small, ((280-small.width)//2, 10), small)
ImageDraw.Draw(startup).text((140,130), 'INICIAR', font=font, fill=(245,190,150), anchor='mm')
startup.save(contents/'startup.png')
(contents/'template.xml').write_text('''<?xml version="1.0" encoding="utf-8"?>
<livearea style="a1" format-ver="01.00" content-rev="1">
  <livearea-background><image>bg0.png</image></livearea-background>
  <gate><startup-image>startup.png</startup-image></gate>
</livearea>
''', encoding='utf-8')
(out/'provenance.json').write_text(json.dumps({'icon': str(args.exe), 'source_icon_size': original.size, 'title': str(logo_source), 'background': str(bg_source)}, indent=2), encoding='utf-8')
print('Extracted icon', original.size, 'and prepared LiveArea in', out)
