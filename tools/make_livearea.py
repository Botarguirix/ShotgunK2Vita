"""Build the Steam app's LiveArea images and boot loader from the owner's art.

Usage: make_livearea.py QUEEN_IMAGE LOGO_IMAGE GAME_EXE OUTPUT_DIR
QUEEN_IMAGE is the queen key art (LiveArea background), LOGO_IMAGE the
"Shotgun King The Final Checkmate" logo (gate button, gets a VITA stamp) and
GAME_EXE shotgun_king.exe, whose icon becomes the bubble. Output follows
private/livearea's layout: icon0.png, pic0.png,
livearea/contents/{bg0.png,startup.png,template.xml}, plus loader.bin, the
960x544 boot screen the app shows while installing/loading.
All PNGs are 8-bit indexed, as the Vita LiveArea expects. Game art stays
private: OUTPUT_DIR belongs under private/ (git-ignored).
"""
import io
import struct
import sys
import zlib
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont, ImageEnhance

PINK, CREAM, NAVY = (231, 24, 80), (250, 204, 172), (17, 22, 41)
FONT = 'C:/Windows/Fonts/impact.ttf'


def save_indexed(img, path):
    q = img.convert('RGB').quantize(colors=255, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    q.info.pop('transparency', None)
    # Pillow writes 4-bit PNGs for art with few colors; the Vita rejects those
    # at install (0x8010113D) and needs 8-bit indexed, so pad the palette.
    pal = q.getpalette()[:768]
    q.putpalette(pal + [0] * (768 - len(pal)))
    q.save(path)
    return q


def cover(img, w, h, focus=0.5):
    """Scale img to fill w x h; focus (0..1 of the width) lands mid-crop."""
    s = max(w / img.width, h / img.height)
    big = img.resize((round(img.width * s), round(img.height * s)), Image.LANCZOS)
    x = min(max(round(big.width * focus) - w // 2, 0), big.width - w)
    y = (big.height - h) // 2
    return big.crop((x, y, x + w, y + h))


def clean_logo(logo):
    """Drop the screenshot's desktop border and paint leftover blue in navy."""
    logo = logo.convert('RGB').crop((8, 8, logo.width - 8, logo.height - 8))
    px = logo.load()
    for y in range(logo.height):
        for x in range(logo.width):
            r, g, b = px[x, y]
            if b > 150 and r < 80:
                px[x, y] = NAVY
    return logo


def vita_logo(logo, height):
    """The logo with a tilted VITA stamp in its colors hanging off its lower
    right corner, below the lettering; height is the whole mark's."""
    logo = clean_logo(logo)
    size = round(height * 0.22)
    s = (height - size) / logo.height
    logo = logo.resize((round(logo.width * s), height - size), Image.LANCZOS)
    font = ImageFont.truetype(FONT, size)
    l, t, r, b = font.getbbox('VITA')
    pad = max(2, size // 8)
    stamp = Image.new('RGBA', (r - l + 4 * pad, b - t + 4 * pad), (0, 0, 0, 0))
    d = ImageDraw.Draw(stamp)
    d.rectangle((0, 0, stamp.width - 1, stamp.height - 1), fill=CREAM)
    d.rectangle((pad, pad, stamp.width - 1 - pad, stamp.height - 1 - pad), fill=NAVY)
    d.text((2 * pad - l, 2 * pad - t), 'VITA', font=font, fill=PINK)
    stamp = stamp.rotate(8, resample=Image.BICUBIC, expand=True)
    out = Image.new('RGB', (logo.width + stamp.width // 2, height), NAVY)
    out.paste(logo, (0, 0))
    out.paste(stamp, (out.width - stamp.width, height - stamp.height), stamp)
    return out


def exe_icon(exe):
    """Largest icon from a PE file's RT_GROUP_ICON/RT_ICON resources."""
    data = Path(exe).read_bytes()
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    nsec = struct.unpack_from('<H', data, pe + 6)[0]
    optsize = struct.unpack_from('<H', data, pe + 20)[0]
    opt = pe + 24
    rsrc_rva = struct.unpack_from('<I', data, opt + (112 if data[opt + 1] == 2 else 96) + 2 * 8)[0]
    sections = []
    for i in range(nsec):
        off = opt + optsize + 40 * i
        vsize, va, rsize, raw = struct.unpack_from('<IIII', data, off + 8)
        sections.append((va, max(vsize, rsize), raw))

    def at(rva):
        for va, size, raw in sections:
            if va <= rva < va + size:
                return rva - va + raw
        raise ValueError('rva outside sections')

    base = at(rsrc_rva)

    def entries(off):
        named, ids = struct.unpack_from('<HH', data, base + off + 12)
        for i in range(named + ids):
            name, target = struct.unpack_from('<II', data, base + off + 16 + 8 * i)
            yield name, target

    def leaf(target):
        while target & 0x80000000:
            target = next(entries(target & 0x7fffffff))[1]
        rva, size = struct.unpack_from('<II', data, base + target)
        return data[at(rva):at(rva) + size]

    types = dict(entries(0))
    icons = {name: leaf(t) for name, t in entries(types[3] & 0x7fffffff)}
    group = leaf(next(entries(types[14] & 0x7fffffff))[1])
    count = struct.unpack_from('<H', group, 4)[0]
    best = None
    for i in range(count):
        w, h, _, _, _, bpp, size, ident = struct.unpack_from('<BBBBHHIH', group, 6 + 14 * i)
        key = (w or 256, bpp)
        if best is None or key > best[0]:
            best = (key, ident)
    blob = icons[best[1]]
    if blob[:8] == b'\x89PNG\r\n\x1a\n':
        return Image.open(io.BytesIO(blob)).convert('RGBA')
    # Plain DIB: wrap it as a single-image .ico so Pillow decodes it.
    w = best[0][0]
    ico = struct.pack('<HHHBBBBHHII', 0, 1, 1, w % 256, w % 256, 0, 0, 1, best[0][1], len(blob), 22) + blob
    return Image.open(io.BytesIO(ico)).convert('RGBA')


def write_loader(img, path):
    """loader.bin: 'SGL1', u16 w, u16 h, 256 RGBA palette, zlib'd indices."""
    q = save_indexed(img, path.with_suffix('.png'))
    pal = q.getpalette()[:768]
    rgba = b''.join(bytes(pal[i * 3:i * 3 + 3]) + b'\xff' for i in range(256))
    path.write_bytes(b'SGL1' + struct.pack('<HH', q.width, q.height) + rgba +
                     zlib.compress(q.tobytes(), 9))
    path.with_suffix('.png').unlink()


def main():
    queen = Image.open(sys.argv[1]).convert('RGB')
    logo = Image.open(sys.argv[2])
    out = Path(sys.argv[4])
    (out / 'livearea/contents').mkdir(parents=True, exist_ok=True)

    # bg0 840x500: the queen key art. The gate box sits at the top center of
    # the LiveArea, so the queen (x ~0.5 of the art) goes right of it: focus
    # 0.31 puts her face at ~720 px, clear of the box. pic0 adds the VITA logo.
    save_indexed(cover(queen, 840, 500, focus=0.31), out / 'livearea/contents/bg0.png')
    pic0 = cover(queen, 960, 544)
    mark = vita_logo(logo, 170)
    pic0.paste(mark, (24, 24))
    save_indexed(pic0, out / 'pic0.png')

    # startup 280x158 (the gate): the system draws its "Start" button over
    # the bottom ~55 px, so the logo and VITA stamp share the top 100 px.
    start = Image.new('RGB', (280, 158), NAVY)
    mark = clean_logo(logo)
    mark = mark.resize((round(mark.width * 96 / mark.height), 96), Image.LANCZOS)
    start.paste(mark, (12, 4))
    font = ImageFont.truetype(FONT, 40)
    l, t, r, b = font.getbbox('VITA')
    stamp = Image.new('RGBA', (r - l + 16, b - t + 16), CREAM)
    d = ImageDraw.Draw(stamp)
    d.rectangle((4, 4, stamp.width - 5, stamp.height - 5), fill=NAVY)
    d.text((8 - l, 8 - t), 'VITA', font=font, fill=PINK)
    stamp = stamp.rotate(8, resample=Image.BICUBIC, expand=True)
    start.paste(stamp, (12 + mark.width + (280 - 12 - mark.width - stamp.width) // 2, 26), stamp)
    save_indexed(start, out / 'livearea/contents/startup.png')

    # icon0 128x128: the Windows game icon (16x16 pixel art, scaled 8x).
    icon = exe_icon(sys.argv[3]).resize((128, 128), Image.NEAREST)
    bubble = Image.new('RGB', (128, 128), NAVY)
    bubble.paste(icon, (0, 0), icon)
    save_indexed(bubble, out / 'icon0.png')

    # loader 960x544: dimmed queen, logo top left and a panel at the bottom
    # where the app draws the game version, status and progress bar
    # (src/main.c, rows 440..520).
    loader = ImageEnhance.Brightness(cover(queen, 960, 544)).enhance(0.55)
    loader.paste(vita_logo(logo, 200), (32, 32))
    d = ImageDraw.Draw(loader)
    d.rectangle((0, 432, 959, 543), fill=NAVY)
    d.rectangle((0, 432, 959, 435), fill=PINK)
    write_loader(loader, out / 'loader.bin')

    (out / 'livearea/contents/template.xml').write_text(
        '<?xml version="1.0" encoding="utf-8"?>\n'
        '<livearea style="a1" format-ver="01.00" content-rev="1">\n'
        '  <livearea-background>\n    <image>bg0.png</image>\n  </livearea-background>\n'
        '  <gate>\n    <startup-image>startup.png</startup-image>\n  </gate>\n'
        '</livearea>\n', encoding='utf-8')
    print('LiveArea written to', out)


if __name__ == '__main__':
    main()
