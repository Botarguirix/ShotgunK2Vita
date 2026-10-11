import sys
import tempfile
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from stability_audit import audit, code_only


class DependencyInventory(unittest.TestCase):
    def test_comments_strings_and_captured_function(self):
        with tempfile.TemporaryDirectory() as temp:
            root=Path(temp);(root/'src').mkdir();(root/'data').mkdir()
            (root/'src/graphics.c').write_text('reg(L,"aspr",aspr);')
            (root/'data/code.lua').write_text("-- sset()\nlocal s='bank()'\n--[=[ delbnk() ]=]\nlocal f=bind(aspr,1,2,3)\ncirc(1,2,3)\n")
            result=audit(root/'data',root)
            states={a['api']:a['status'] for a in result['apis']}
            self.assertEqual(states,{'aspr':'partial','circ':'missing'})
            self.assertNotIn('delbnk',code_only('--[=[delbnk()]=]'))

if __name__=='__main__': unittest.main()
