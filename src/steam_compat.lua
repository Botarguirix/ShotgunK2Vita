-- Steam 1.623b -> legacy C-runtime API bridge. No game scripts are patched.
-- Puente experimental; sin servicios online ni vigilancia de archivos en vivo.
_S=_S or {print=print,bprint=bprint,log=log,concat=table.concat}
wlog=wlog or log
local assets=assert(SG_STEAM_ASSETS,'Steam profile required')
local function canonical(s) return (s:gsub('^%.%./',''):gsub('^%.%./','')) end
local function stem(s) return s:match('([^/]+)%.[^%.]+$') or s end
local function matching(pattern)
  pattern=canonical(pattern):gsub('<%*>','*')
  local escaped=pattern:gsub('([%^%$%(%)%%%.%[%]%+%-%?])','%%%1'):gsub('%*','.*')
  local out={}
  for _,name in ipairs(assets.files) do if name:match('^'..escaped..'$') then out[#out+1]=name end end
  return out
end
local old_surface=newsrf
function newsrf(a,b,c)
  if type(a)=='string' and type(b)=='number' then return old_surface(b,c,a) end
  if type(a)=='string' and type(b)=='string' then return old_surface(canonical(b),a) end
  if type(a)=='string' and a:find('<*>',1,true) then
    for _,path in ipairs(matching(a)) do old_surface(path,stem(path)) end
    return
  end
  return old_surface(a,b,c)
end
local old_font=newfnt
function newfnt(name,path,size) return old_font(canonical(path),size,name) end
local old_music,old_fx=newmus,newsfx
local function music_path(path) path=canonical(path);return assets.music[path] or path end
function newmus(name,path,volume)
  if not path and name:find('<*>',1,true) then
    for _,file in ipairs(matching(name)) do old_music(music_path(file),stem(file)) end
    return
  end
  if not path then path=name;name=stem(path) end
  return old_music(music_path(path),name,volume)
end
function newsfx(name,path,volume)
  if not path then path=name;name=stem(path) end
  return old_fx(canonical(path),name,volume)
end
function watch(pattern,callback,mode)
  if type(pattern)=='boolean' then return false end
  assert(type(callback)=='function','Only one-shot asset watch is supported')
  for _,file in ipairs(matching(pattern)) do callback(file,stem(file)) end
end
function unwatch() end -- No persistent host filesystem watch is installed.
function psyms() end -- Optional development overlay, not gameplay logic.
SG_STEAM_BRIDGE={version=1,watch_mode='one-shot',source='Steam SGR'}

-- Isolated diagnostic storage. This is NOT Steam save encryption support.
-- Sólo perfiles nuevos creados por prepare_steam_data; nunca importar saves PC.
local old_file,old_afile=file,afile
local function diagnostic_path(path)
  path=path:gsub('^%./','')
  if path:match('^settings%.txt') then return 'save/steam-'..path end
  return path
end
function file(path,data)
  path=diagnostic_path(path)
  if data==nil then return old_file(path) end
  return old_file(path,data)
end
function afile(path,data,encoded)
  return old_afile(diagnostic_path(path),data,false)
end
function encfile(path) return old_file(diagnostic_path(path)) end
SG_STEAM_BRIDGE.save_format='plain diagnostic only; not compatible with Steam encoded saves'

-- Headless/Vita backend has no desktop text events. Preserve an explicit queue
-- instead of inventing key presses; a future platform layer can feed it.
local typed={}
function SG_STEAM_BRIDGE.feed_text(text)
  for i=1,#text do typed[#typed+1]=text:sub(i,i) end
end
function catchtxt() return table.remove(typed,1) end
SG_STEAM_BRIDGE.text_input='explicit ASCII queue; no desktop keyboard hook'
