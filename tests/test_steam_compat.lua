-- Synthetic bridge contract tests: no commercial code or resources.
local calls={}
local function capture(kind)
 return function(...) calls[#calls+1]={kind,...};return select(1,...) end
end
file=capture('file');afile=capture('async');
newsrf=capture('surface');newfnt=capture('font');newmus=capture('music');newsfx=capture('effect')
print=capture('print');bprint=capture('bprint');log=capture('log')
SG_STEAM_ASSETS={files={'assets/gfx/gfx.png','assets/gfx/cards.png','assets/music/theme.ogg','assets/sfx/click.wav'},music={['assets/music/theme.ogg']='assets/music/theme.mp3'}}
dofile('src/steam_compat.lua')
newsrf('board',320,180)
assert(calls[#calls][1]=='surface' and calls[#calls][2]==320 and calls[#calls][3]==180 and calls[#calls][4]=='board')
newsrf('sheet','assets/gfx/gfx.png')
assert(calls[#calls][2]=='assets/gfx/gfx.png' and calls[#calls][3]=='sheet')
local n=#calls;newsrf('assets/gfx/<*>.png');assert(#calls==n+2)
newfnt('small','assets/fonts/font.ttf',12);assert(calls[#calls][2]=='assets/fonts/font.ttf' and calls[#calls][3]==12 and calls[#calls][4]=='small')
newmus('assets/music/<*>.ogg');assert(calls[#calls][2]=='assets/music/theme.mp3' and calls[#calls][3]=='theme')
newsfx('tap','../../assets/sfx/click.wav');assert(calls[#calls][2]=='assets/sfx/click.wav' and calls[#calls][3]=='tap')
local count=0;watch('assets/sfx/<*>.wav',function(path,name)count=count+1;assert(name=='click' and path=='assets/sfx/click.wav')end,2);assert(count==1)
assert(watch(false)==false)
assert(_S.print and _S.bprint and _S.log and _S.concat({'a','b'})=='ab')
file('./settings.txt','settings');assert(calls[#calls][2]=='save/steam-settings.txt')
afile('save/reg.sav','contents',true);assert(calls[#calls][2]=='save/reg.sav' and calls[#calls][4]==false)
assert(SG_STEAM_BRIDGE.save_format:find('not compatible',1,true))
SG_STEAM_BRIDGE.feed_text('AB');assert(catchtxt()=='A' and catchtxt()=='B' and catchtxt()==nil)
io.write('STEAM_COMPAT_CONTRACTS_PASSED\n')
