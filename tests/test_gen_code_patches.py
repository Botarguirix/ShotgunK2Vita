import importlib.util
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('patchgen',ROOT/'tools/gen_code_patches.py')
gen=importlib.util.module_from_spec(spec);spec.loader.exec_module(gen)

class PrivatePatchGeneration(unittest.TestCase):
    def test_empty_without_private_files(self):
        with tempfile.TemporaryDirectory() as td:
            out=Path(td)/'generated/code_patches.h'
            subprocess.run([sys.executable,str(ROOT/'tools/gen_code_patches.py'),'--empty',str(out)],check=True,capture_output=True)
            self.assertIn('#define SG_CODE_PATCH_COUNT 0',out.read_text())
            self.assertIn('{0,0}',out.read_text())
    def test_ordered_replacements(self):
        before='first\nvalue=1\nlast'; after='first\nvalue=2\nlast'
        patches=gen.hunks(before.split('\n'),after.split('\n'))
        self.assertEqual(gen.apply(before,patches),after)
    def test_wrong_target_does_not_write_header(self):
        with tempfile.TemporaryDirectory() as td:
            base=Path(td)
            for name,txt in [('pre','a\nb\nc'),('opt','a\nnew\nc'),('target','different')]:
                (base/name).write_text(txt)
            out=base/'generated.h'
            proc=subprocess.run([sys.executable,str(ROOT/'tools/gen_code_patches.py'),str(base/'pre'),str(base/'opt'),str(base/'target'),str(out)],capture_output=True)
            self.assertNotEqual(proc.returncode,0);self.assertFalse(out.exists())
    def test_private_generation_has_explicit_count(self):
        with tempfile.TemporaryDirectory() as td:
            base=Path(td)
            for name,txt in [('pre','a\nb\nc'),('opt','a\nnew\nc'),('target','a\nb\nc')]:
                (base/name).write_text(txt)
            out=base/'generated.h'
            subprocess.run([sys.executable,str(ROOT/'tools/gen_code_patches.py'),str(base/'pre'),str(base/'opt'),str(base/'target'),str(out)],check=True,capture_output=True)
            self.assertIn('#define SG_CODE_PATCH_COUNT 1',out.read_text())

if __name__=='__main__':unittest.main()
