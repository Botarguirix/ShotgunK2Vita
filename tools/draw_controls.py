"""Draw an exact, code-derived Vita control diagram (no game artwork)."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

root = Path(__file__).resolve().parents[1]
out = root/'docs/images'
out.mkdir(parents=True, exist_ok=True)
im = Image.new('RGB',(1600,1200),(14,18,33))
d = ImageDraw.Draw(im)
font_root = Path('C:/Windows/Fonts')
def font(size, bold=False):
    return ImageFont.truetype(str(font_root/('segoeuib.ttf' if bold else 'segoeui.ttf')),size)
peach=(255,208,173); pink=(255,103,145); muted=(169,173,194)
d.text((60,38),'ShotgunK2Vita · Controles PS Vita',font=font(46,True),fill=peach)
d.text((62,103),'Mapa adaptado a Vita OLED · versión 00.14 · sólo L1/R1 usan touch',font=font(24),fill=muted)

# Front view of an OLED Vita: these are schematic shapes, not a device photo.
d.rounded_rectangle((360,280,1240,730),radius=110,fill=(10,12,18),outline=(91,94,118),width=5)
d.rounded_rectangle((523,323,1077,660),radius=15,fill=(26,31,49),outline=(100,100,120),width=3)
for y in range(8):
    for x in range(8):
        d.rectangle((680+x*29,374+y*29,708+x*29,402+y*29),fill=(104,99,133) if (x+y)%2 else (215,172,143))
d.text((800,478),'SGK',font=font(36,True),fill=(14,18,33),anchor='mm')
d.text((800,695),'PS VITA',font=font(20,True),fill=(130,135,157),anchor='mm')
for x,y in ((445,592),(1154,592)):
    d.ellipse((x-36,y-36,x+36,y+36),fill=(25,27,37),outline=(116,117,137),width=3)
    d.ellipse((x-22,y-22,x+22,y+22),fill=(40,43,57))
d.rounded_rectangle((427,385,463,479),radius=5,fill=(44,47,64),outline=(124,127,148),width=2)
d.rounded_rectangle((399,413,491,451),radius=5,fill=(44,47,64),outline=(124,127,148),width=2)
for x,y,symbol in ((1154,382,'▲'),(1204,432,'○'),(1154,482,'×'),(1104,432,'□')):
    d.ellipse((x-22,y-22,x+22,y+22),fill=(40,43,57),outline=(121,122,146),width=2)
    d.text((x,y-2),symbol,font=font(32),fill=pink,anchor='mm')
for x,label in ((459,'SELECT'),(1144,'START')):
    d.rounded_rectangle((x-30,669,x+30,684),radius=6,fill=(65,67,82))
    d.text((x,706),label,font=font(13,True),fill=muted,anchor='mm')
for x,label in ((433,'L'),(1166,'R')):
    d.rounded_rectangle((x-59,250,x+59,277),radius=10,fill=(77,78,96),outline=muted,width=2)
    d.text((x,262),label,font=font(18,True),fill=peach,anchor='mm')

def card(box,title,lines,target):
    x,y,w,h=box
    d.rounded_rectangle((x,y,x+w,y+h),radius=16,fill=(32,37,59),outline=(66,70,94),width=2)
    d.text((x+18,y+13),title,font=font(23,True),fill=peach)
    for i,line in enumerate(lines):d.text((x+18,y+49+i*29),line,font=font(20),fill=muted)
    if target:
        start=(x+w,y+h//2) if x<800 else (x,y+h//2)
        elbow=(start[0]+25 if x<800 else start[0]-25,start[1])
        d.line((start,elbow,target),fill=pink,width=3);d.ellipse((target[0]-4,target[1]-4,target[0]+4,target[1]+4),fill=pink)
card((35,184,280,123),'L',['Mantener: movimiento','arriesgado'],(405,263))
card((1285,184,280,123),'R',['Disparar','(R2 del esquema PS)'],(1190,263))
card((35,352,280,157),'Cruceta / stick izq.',['Navegar y seleccionar','destino de movimiento'],(435,435))
card((1285,350,280,207),'Botones',['×  Confirmar / mover','○  Cancelar','□  Acción especial','Triángulo: recargar'],(1185,432))
card((35,557,280,170),'SELECT / START',['SELECT: información','START: pausa'],(459,675))
card((1285,592,280,123),'Stick derecho',['Apuntar','con la escopeta'],(1154,592))

d.text((60,793),'Zonas táctiles activas y salida',font=font(28,True),fill=peach)
items=[('L1 táctil','Esquina inferior izquierda'),('R1 táctil','Esquina inferior derecha'),('Códice / páginas','L1: anterior · R1: siguiente'),('Salir del juego','Menú → Exit'),('Si aparece un error','START: salir y conservar logs')]
for i,(keys,label) in enumerate(items):
    col=i%2;row=i//2;x=60+col*755;y=849+row*84
    d.rounded_rectangle((x,y,x+725,y+68),radius=13,fill=(32,37,59))
    d.text((x+16,y+10),keys,font=font(20,True),fill=pink)
    d.text((x+295,y+14),label,font=font(19),fill=peach)
for x,label in ((531,'L1'),(1008,'R1')):
    d.rectangle((x,618,x+59,650),fill=(56,40,32),outline=pink,width=2)
    d.text((x+30,635),label,font=font(20,True),fill=peach,anchor='mm')
d.text((60,1138),'Cruceta/stick seleccionan; × confirma movimiento. Sin atajos SELECT ni toque del tablero.',font=font(21),fill=muted)
im.save(out/'controls-vita.png')
print(out/'controls-vita.png')
