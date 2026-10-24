"""Prepare Vita PNG-8 LiveArea assets from external artwork."""
import argparse
from pathlib import Path
from PIL import Image, ImageOps

def indexed(image: Image.Image, path: Path) -> None:
    image = image.convert("RGB").quantize(colors=128, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    image.info.clear()
    image.save(path, bits=8, optimize=False)

def main() -> None:
    p = argparse.ArgumentParser()
    p.add_argument("icon", type=Path)
    p.add_argument("background", type=Path)
    p.add_argument("--out", type=Path, default=Path("private/livearea"))
    a = p.parse_args(); root = a.out; contents = root / "livearea" / "contents"
    contents.mkdir(parents=True, exist_ok=True)
    icon = Image.open(a.icon).convert("RGB")
    bg = Image.open(a.background).convert("RGB")
    indexed(ImageOps.fit(icon, (128, 128), method=Image.Resampling.LANCZOS), root / "icon0.png")
    indexed(ImageOps.fit(bg, (840, 500), method=Image.Resampling.LANCZOS), contents / "bg0.png")
    indexed(ImageOps.fit(bg, (960, 544), method=Image.Resampling.LANCZOS), root / "pic0.png")
    startup = Image.new("RGB", (280, 158), (0, 29, 35))
    startup.paste(ImageOps.fit(icon, (104, 104), method=Image.Resampling.LANCZOS), (88, 8))
    indexed(startup, contents / "startup.png")
    xml = '''<?xml version="1.0" encoding="utf-8"?>
<livearea style="psmobile" format-ver="01.00" content-rev="6">
  <livearea-background><image>bg0.png</image></livearea-background>
  <gate><startup-image>startup.png</startup-image></gate>
</livearea>
'''
    (contents / "template.xml").write_bytes(xml.replace("\n", "\r\n").encode())
    for f in root.rglob("*.png"):
        with Image.open(f) as im:
            assert im.mode == "P" and f.stat().st_size < 420 * 1024
            print(f, im.size, f.stat().st_size)

if __name__ == "__main__":
    main()
