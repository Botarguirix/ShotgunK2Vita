"""Check ZIP integrity, SFO identity and indexed LiveArea PNGs before delivery."""
import argparse
import json
import struct
import xml.etree.ElementTree as ET
import zipfile
import zlib

SIZES = {'icon0.png': (128, 128), 'pic0.png': (960, 544),
         'livearea/contents/bg0.png': (840, 500),
         'livearea/contents/startup.png': (280, 158)}


def png_info(data, expected):
    if data[:8] != b'\x89PNG\r\n\x1a\n':
        raise ValueError('Invalid PNG signature')
    chunks, cursor, compressed = {}, 8, bytearray()
    while cursor < len(data):
        length = struct.unpack_from('>I', data, cursor)[0]
        kind = data[cursor+4:cursor+8]
        body = data[cursor+8:cursor+8+length]
        end = cursor+12+length
        if end > len(data):
            raise ValueError('Truncated PNG chunk')
        crc = struct.unpack_from('>I', data, cursor+8+length)[0]
        if zlib.crc32(kind+body) & 0xffffffff != crc:
            raise ValueError('Invalid PNG CRC')
        chunks[kind] = body
        if kind == b'IDAT':
            compressed.extend(body)
        cursor = end
        if kind == b'IEND':
            break
    width, height, bits, color, compression, filtering, interlace = struct.unpack('>IIBBBBB', chunks[b'IHDR'])
    if (width, height) != expected or (bits, color, compression, filtering, interlace) != (8, 3, 0, 0, 0):
        raise ValueError(f'Unsupported Vita PNG: {(width, height, bits, color, interlace)}')
    if len(chunks.get(b'PLTE', b'')) != 768 or b'tRNS' in chunks:
        raise ValueError('Expected opaque 256-entry palette')
    raw = zlib.decompress(compressed)
    if len(raw) != height*(width+1) or any(raw[y*(width+1)] > 4 for y in range(height)):
        raise ValueError('Invalid PNG scanlines')
    if b'IEND' not in chunks or cursor != len(data):
        raise ValueError('Invalid PNG ending')
    return {'width': width, 'height': height, 'bit_depth': bits,
            'color_type': color, 'palette_entries': 256, 'transparency': False}


def sfo_info(data):
    magic, version, keys, values, count = struct.unpack_from('<5I', data)
    if magic != 0x46535000:
        raise ValueError('Invalid SFO')
    result = {}
    for i in range(count):
        key, fmt, length, capacity, value = struct.unpack_from('<HHIII', data, 20+16*i)
        name = data[keys+key:].split(b'\0', 1)[0].decode()
        body = data[values+value:values+value+length]
        result[name] = body.rstrip(b'\0').decode() if fmt == 0x0204 else body.hex()
    return result


def validate(path, require_art=False):
    with zipfile.ZipFile(path) as archive:
        if archive.testzip() is not None:
            raise ValueError('VPK ZIP CRC failed')
        names = archive.namelist()
        if len(names) != len(set(names)):
            raise ValueError('Duplicate VPK entries')
        archive.read('eboot.bin')
        sfo = sfo_info(archive.read('sce_sys/param.sfo'))
        if sfo.get('TITLE_ID') not in {'SGKV00001', 'SGKV00002', 'SGK550001'}:
            raise ValueError('Unexpected title ID')
        images = {}
        template = 'sce_sys/livearea/contents/template.xml'
        if require_art or template in names:
            for name, size in SIZES.items():
                images[name] = png_info(archive.read('sce_sys/'+name), size)
            xml = ET.fromstring(archive.read(template))
            for node in xml.iter():
                if node.tag in {'image', 'startup-image'}:
                    archive.read('sce_sys/livearea/contents/'+node.text.strip())
        return {'sfo': sfo, 'images': images, 'zip_crc': 'passed'}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('vpk')
    parser.add_argument('--require-art', action='store_true')
    args = parser.parse_args()
    print(json.dumps(validate(args.vpk, args.require_art), indent=2))
