"""Generate original text-only LiveArea assets for the alpha release."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

root = Path(__file__).resolve().parents[1] / 'private/livearea'
for name, size, text in [('icon0.png',(128,128),'SGK\nVITA'),('pic0.png',(960,544),'SHOTGUN KING\n0.78 ALPHA'),('livearea/contents/bg0.png',(840,500),'SHOTGUN KING\n0.78 ALPHA'),('livearea/contents/startup.png',(280,158),'INICIAR / START')]:
    im=Image.new('RGB',size,(15,25,45));d=ImageDraw.Draw(im)
    d.rectangle((3,3,size[0]-4,size[1]-4),outline=(85,160,220),width=3)
    font=ImageFont.load_default(size=max(12,min(32,size[0]//9)))
    d.multiline_text((size[0]//2,size[1]//2),text,font=font,fill=(235,240,250),anchor='mm',align='center')
    p=root/name;p.parent.mkdir(parents=True,exist_ok=True)
    indexed=im.quantize(colors=256)
    palette=indexed.getpalette()
    indexed.putpalette((palette+[0]*768)[:768])
    indexed.save(p)
(root/'livearea/contents/template.xml').write_text('<?xml version="1.0" encoding="utf-8"?><livearea style="a1" format-ver="01.00" content-rev="1"><livearea-background><image>bg0.png</image></livearea-background><gate><startup-image>startup.png</startup-image></gate></livearea>',encoding='utf-8')
