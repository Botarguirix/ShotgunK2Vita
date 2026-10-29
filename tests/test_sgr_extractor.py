"""Synthetic checks for the recovered SGR reader; no commercial fixtures."""
import importlib.util
import struct
import tempfile
import unittest
import zlib
from pathlib import Path

spec = importlib.util.spec_from_file_location('extract_sgr', Path(__file__).resolve().parents[1]/'tools/extract_sgr.py')
sgr = importlib.util.module_from_spec(spec)
spec.loader.exec_module(sgr)


def wrap(payload, seed=0x12345678):
    trailer = (seed ^ 0x550F0F55).to_bytes(4, 'big')
    return sgr.transform(payload+trailer)+trailer


def text(s):
    raw=s.encode('utf-8')
    return struct.pack('>H',len(raw))+raw


def container(entries):
    raw=text('Synthetic fixture')+text('Test author')+text('main.lua')
    for name, flag, data in entries:
        raw+=text(name)+bytes([flag])+struct.pack('>I',len(data))+data
    return raw


class SgrTests(unittest.TestCase):
    def test_native_verified_vector(self):
        # Obtained independently by emulating the original x86 transform.
        trailer=(0x12345678 ^ 0x550F0F55).to_bytes(4,'big')
        self.assertEqual(sgr.transform(bytes(range(16))+trailer).hex(), 'a101f103cb05f407b409140b420ddd0f')

    def test_short_trailer(self):
        with self.assertRaises(ValueError):sgr.transform(b'abc')

    def test_zlib_roundtrip(self):
        raw=container([('main.lua',0,wrap(b'-- synthetic only\n'))])
        self.assertEqual(sgr.unpack(wrap(struct.pack('>I',len(raw))+zlib.compress(raw)),1024),raw)

    def test_size_limit(self):
        with self.assertRaises(ValueError):sgr.unpack(wrap(struct.pack('>I',10000)+zlib.compress(b'hello')),100)

    def test_wrong_declared_size(self):
        with self.assertRaises(ValueError):sgr.unpack(wrap(struct.pack('>I',4)+zlib.compress(b'hello')),100)

    def test_trailing_compressed_data(self):
        with self.assertRaises(ValueError):sgr.unpack(wrap(struct.pack('>I',5)+zlib.compress(b'hello')+b'x'),100)

    def test_truncated_record(self):
        with self.assertRaises(ValueError):sgr.parse_container(container([('main.lua',0,wrap(b'abc'))])[:-1])

    def test_safe_paths(self):
        for name in ['../outside.lua','C:/outside.lua','a\\b.lua','/absolute.lua','CON.txt','trailing.','a/../b.lua']:
            with self.subTest(name=name),self.assertRaises(ValueError):sgr.safe_relative(name)

    def test_extract_virtual_duplicate_and_wildcard(self):
        raw=container([('main.lua',0,wrap(b'-- first\n')),('main.lua',0,wrap(b'-- second\n')),
                       ('../../libs/helper.lua',0,wrap(b'-- helper\n')),('gfx/*.png',1,wrap(b'list'))])
        with tempfile.TemporaryDirectory() as temp:
            root=Path(temp)/'output';record=sgr.extract(raw,root,{'kind':'synthetic'})
            self.assertEqual((root/'files/main.lua').read_bytes(),b'-- first\n')
            self.assertEqual((root/'duplicates/entry-0001.lua').read_bytes(),b'-- second\n')
            self.assertEqual((root/'virtual/entry-0002.lua').read_bytes(),b'-- helper\n')
            self.assertEqual((root/'lists/entry-0003.bin').read_bytes(),b'list')
            self.assertEqual(len(record['entries']),4)
            self.assertFalse((Path(temp)/'libs').exists())
            with self.assertRaises(FileExistsError):sgr.extract(raw,root,{})


if __name__ == '__main__':unittest.main(verbosity=2)
