import sys,tempfile,unittest,json,hashlib
from pathlib import Path
from PIL import Image
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from prepare_mod import install_mod

class ModPreparationTests(unittest.TestCase):
 def setup_data(self,root):
  game=root/'game';p=game/'assets/gfx';p.mkdir(parents=True)
  im=Image.new('P',(1,1));im.putpalette([c for i in range(256) for c in (i,i,i)]);im.save(p/'gfx.png')
  mod=root/'source';mod.mkdir();(mod/'info.lua').write_text('name="sample"\n',encoding='utf-8')
  (mod/'script.lua').write_text('-- synthetic Steam\n',encoding='utf-8');(mod/'script1.5.lua').write_text('-- synthetic legacy\n',encoding='utf-8')
  return game,mod
 def test_steam_preserves_final_checkmate_script(self):
  with tempfile.TemporaryDirectory() as t:
   game,mod=self.setup_data(Path(t));r=install_mod(mod,game,'steam',True)
   self.assertEqual(r['selected_script'],'script.lua');self.assertEqual((game/'mods/sample/script.lua').read_bytes(),(mod/'script.lua').read_bytes())
   self.assertIn('"sample", true',(game/'mods/modlist.lua').read_text())
 def test_legacy_selects_15(self):
  with tempfile.TemporaryDirectory() as t:
   game,mod=self.setup_data(Path(t));r=install_mod(mod,game)
   self.assertEqual(r['selected_script'],'script1.5.lua');self.assertEqual((game/'mods/sample/script.lua').read_bytes(),(mod/'script1.5.lua').read_bytes())
 def test_replace_is_explicit_and_keeps_backup(self):
  with tempfile.TemporaryDirectory() as t:
   game,mod=self.setup_data(Path(t));install_mod(mod,game,'steam');target=game/'mods/sample/script.lua';original=target.read_bytes()
   with self.assertRaises(FileExistsError):install_mod(mod,game,'steam')
   (mod/'script.lua').write_text('-- changed fixture\n');r=install_mod(mod,game,'steam',replace=True)
   self.assertEqual((Path(r['backup'])/'script.lua').read_bytes(),original)
 def test_steam_does_not_fall_back_to_legacy(self):
  with tempfile.TemporaryDirectory() as t:
   game,mod=self.setup_data(Path(t));(mod/'script.lua').unlink()
   with self.assertRaises(ValueError):install_mod(mod,game,'steam')
 def test_unsafe_name_does_not_install(self):
  with tempfile.TemporaryDirectory() as t:
   game,mod=self.setup_data(Path(t));(mod/'info.lua').write_text('name="../outside"')
   with self.assertRaises(ValueError):install_mod(mod,game,'steam')
   self.assertFalse((game.parent/'outside').exists())

if __name__=='__main__':unittest.main(verbosity=2)
