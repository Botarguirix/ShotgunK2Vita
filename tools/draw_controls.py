"""Original bilingual Vita diagrams; system fonts only, no game assets."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

# Hardware layout reference only; no image is copied:
# https://manuals.playstation.net/document/en/psvita/basic/partnames.html
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'docs/images'
BG='#101827'; PANEL='#1d2a40'; INK='#edf2fa'; MUTED='#bccbdf'; ACCENT='#67d5e7'

def font(n,bold=False):
    candidates=[Path('C:/Windows/Fonts')/('segoeuib.ttf' if bold else 'segoeui.ttf'),Path('/usr/share/fonts/truetype/dejavu')/('DejaVuSans-Bold.ttf' if bold else 'DejaVuSans.ttf')]
    for p in candidates:
        if p.is_file():return ImageFont.truetype(str(p),n)
    return ImageFont.load_default(size=n)

def draw(lang):
    es=lang=='es'
    im=Image.new('RGB',(1600,1420),BG);d=ImageDraw.Draw(im)
    d.text((60,35),'ShotgunKing2Vita',font=font(46,True),fill=INK)
    d.text((60,96),'0.78 alpha | '+('Controles y gestos touch' if es else 'Controls and touch gestures'),font=font(28),fill=ACCENT)
    d.text((60,145),'Esquema propio. Steam: touch frontal activo por defecto.' if es else 'Original schematic. Steam: front touch enabled by default.',font=font(23),fill=MUTED)
    # All geometry is drawn here; no device photos or game screen.
    d.rounded_rectangle((345,300,1255,750),100,fill='#080e18',outline='#7c91b0',width=4)
    d.rounded_rectangle((515,345,1085,670),18,fill=PANEL,outline='#5d7290',width=3)
    d.text((800,480),'TOUCH',font=font(42,True),anchor='mm',fill=ACCENT)
    d.text((800,535),'1 dedo / 2 dedos' if es else '1 finger / 2 fingers',font=font(24),anchor='mm',fill=MUTED)
    for x in [445,1155]:
        d.ellipse((x-34,574,x+34,642),fill=PANEL,outline=MUTED,width=3)
    d.rectangle((428,383,462,477),fill=MUTED);d.rectangle((398,413,492,447),fill=MUTED)
    for x,y,label in [(1155,390,'TRI'),(1200,435,'O'),(1155,480,'X'),(1110,435,'SQ')]:
        d.ellipse((x-22,y-22,x+22,y+22),outline=MUTED,width=2)
        if label=='TRI':d.polygon([(x,y-12),(x-12,y+10),(x+12,y+10)],outline=INK,width=2)
        elif label=='SQ':d.rectangle((x-10,y-10,x+10,y+10),outline=INK,width=2)
        else:d.text((x,y),label,font=font(24,True),anchor='mm',fill=INK)
    for x,label in [(435,'L'),(1165,'R')]:
        d.rounded_rectangle((x-58,268,x+58,300),10,fill=PANEL,outline=ACCENT,width=2);d.text((x,284),label,font=font(22,True),anchor='mm',fill=INK)
    for x,label in [(1140,'SELECT'),(1210,'START')]:
        d.rounded_rectangle((x-25,683,x+25,703),6,fill=PANEL,outline=MUTED);d.text((x,720),label,font=font(14),anchor='mm',fill=MUTED)
    d.ellipse((427,682,463,718),outline=MUTED,width=2)
    d.text((445,700),'PS',font=font(13),anchor='mm',fill=MUTED)
    def box(x,y,w,h,title,body,target=None):
        d.rounded_rectangle((x,y,x+w,y+h),16,fill=PANEL)
        d.text((x+17,y+13),title,font=font(24,True),fill=ACCENT)
        d.multiline_text((x+17,y+50),body,font=font(22),fill=INK,spacing=6)
        if target:
            a=(x+w,y+h//2) if x<800 else (x,y+h//2)
            d.line([a,target],fill=ACCENT,width=2)
    box(45,240,270,125,'L / L2','Acción L2 del juego' if es else 'Game L2 action',(420,284))
    box(1285,240,270,125,'R / R2','Disparar' if es else 'Shoot',(1180,284))
    box(45,395,270,150,'Cruceta / stick izq.' if es else 'D-pad / left stick','Navegar / mover\nX confirma destino' if es else 'Navigate / move\nX confirms destination',(445,435))
    box(1285,395,270,205,'Botones' if es else 'Buttons','X: confirmar\nO: atrás\nTriángulo: recargar\nCuadrado: especial' if es else 'X: confirm\nO: back\nTriangle: reload\nSquare: special',(1200,435))
    box(45,580,270,155,'SELECT / START','Información / pausa' if es else 'Information / pause')
    box(1285,610,270,125,'Stick derecho' if es else 'Right stick','Apuntar' if es else 'Aim',(1155,608))
    d.text((60,795),'Gestos en la pantalla frontal' if es else 'Front touchscreen gestures',font=font(31,True),fill=INK)
    box(60,850,470,170,'1. Mover cursor' if es else '1. Move cursor','Un dedo en movimiento.\nNo pulsa el botón del ratón.' if es else 'Move one finger.\nDoes not press a mouse button.')
    box(565,850,470,170,'2. Arrastrar' if es else '2. Drag','Mantén quieto ~0.33 s*\ny después mueve el dedo.' if es else 'Hold still for ~0.33 s*\nthen move your finger.')
    box(1070,850,470,170,'3. Clic izquierdo' if es else '3. Left click','Dos dedos sobre la pantalla.\nSuelta para finalizar el clic.' if es else 'Place two fingers on screen.\nRelease to end the click.')
    d.text((60,1045),'*20 fotogramas: ~0.33 s a 60 FPS; depende de la tasa real.' if es else '*20 frames: ~0.33 s at 60 FPS; actual duration depends on frame rate.',font=font(22),fill=MUTED)
    d.text((60,1080),'Sin gesto de clic derecho. Tras un arrastre, levanta todos los dedos.' if es else 'No right-click gesture. Lift all fingers after a drag.',font=font(22),fill=MUTED)
    box(60,1140,730,150,'Panel trasero: L1 / R1' if es else 'Rear touch: L1 / R1','Zona superior izq.: página anterior (L1).\nZona superior der.: página siguiente (R1).' if es else 'Upper left zone: previous page (L1).\nUpper right zone: next page (R1).')
    box(825,1140,715,150,'Configuración' if es else 'Configuration','front_touch=1 activa el ratón táctil.\nApp 1.5: desactivado por defecto.' if es else 'front_touch=1 enables touch mouse.\nApp 1.5: disabled by default.')
    d.text((60,1335),'L/R físicos = L2/R2. El panel trasero no sustituye el disparo R2.' if es else 'Physical L/R = L2/R2. Rear touch does not replace the R2 shooting button.',font=font(24),fill=ACCENT)
    OUT.mkdir(parents=True,exist_ok=True);im.save(OUT/f'controls_{lang}.png')

if __name__=='__main__':
    for lang in ['es','en']:draw(lang)
