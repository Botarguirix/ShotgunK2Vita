"""Extract the investigated SUGAR data.sgr format without executing game code.

Extrae el formato SUGAR investigado sin ejecutar el juego. Original game data
must remain private / Los recursos originales deben permanecer privados.
"""
import argparse
import hashlib
import json
import re
import struct
import zlib
from pathlib import Path, PurePosixPath

MASK = 0xFFFFFFFF


def transform(blob):
    """Undo the even-byte XOR layer; the final four bytes store the seed."""
    if len(blob) < 4:
        raise ValueError('Truncated seed trailer / Semilla truncada')
    seed = int.from_bytes(blob[-4:], 'big') ^ 0x550F0F55
    x = seed
    y = ((seed >> 1) + 0x159A55E5) & MASK
    z = ((seed >> 3) + 0x1F123BB5) & MASK
    w = ((seed >> 5) + 0x05491333) & MASK
    v = ((seed >> 7) + 0x34DAD465) & MASK
    # One result selects how many additional outputs to discard.
    t = x ^ (x >> 7)
    nv = (v ^ (v << 6) ^ t ^ (t << 13)) & MASK
    skip = ((2 * z + 1) * nv) & 15
    x, y, z, w, v = y, z, w, v, nv
    for _ in range(skip):
        t = x ^ (x >> 7)
        nv = (v ^ (v << 6) ^ t ^ (t << 13)) & MASK
        x, y, z, w, v = y, z, w, v, nv
    out = bytearray(blob[:-4])
    for i in range(0, len(out), 2):
        t = x ^ (x >> 7)
        nv = (v ^ (v << 6) ^ t ^ (t << 13)) & MASK
        out[i] ^= ((2 * z + 1) * nv) & 255
        x, y, z, w, v = y, z, w, v, nv
    return bytes(out)


def unpack(blob, max_bytes):
    decoded = transform(blob)
    if len(decoded) < 6:
        raise ValueError('Truncated compression header / Cabecera truncada')
    expected = int.from_bytes(decoded[:4], 'big')
    if not 0 < expected <= max_bytes:
        raise ValueError('Decoded size exceeds limit / Tamaño fuera de límite')
    dec = zlib.decompressobj()
    raw = dec.decompress(decoded[4:], expected + 1)
    if len(raw) != expected or not dec.eof or dec.unused_data or dec.unconsumed_tail:
        raise ValueError('Incomplete or inconsistent zlib stream / Stream zlib inconsistente')
    return raw


def parse_container(raw):
    pos = 0

    def take(n):
        nonlocal pos
        if n < 0 or pos + n > len(raw):
            raise ValueError('Container field out of bounds / Campo fuera de límites')
        value = raw[pos:pos+n]
        pos += n
        return value

    def string():
        n = int.from_bytes(take(2), 'big')
        return take(n).decode('utf-8')

    metadata = [string() for _ in range(3)]
    entries = []
    while pos < len(raw):
        name = string()
        flag = take(1)[0]
        size = int.from_bytes(take(4), 'big')
        offset = pos
        if flag not in (0, 1) or size < 4:
            raise ValueError('Unsupported entry / Entrada no admitida: ' + name)
        take(size)
        entries.append({'name': name, 'flag': flag, 'stored_bytes': size, 'offset': offset})
    return metadata, entries


def safe_relative(name):
    # Reject Windows aliases too, even if extraction runs on Unix.
    if not name or '\\' in name or '\x00' in name or ':' in name:
        raise ValueError('Unsafe entry name / Nombre inseguro: ' + repr(name))
    p = PurePosixPath(name)
    if p.is_absolute() or any(c in ('', '.', '..') for c in name.split('/')):
        raise ValueError('Unsafe entry path / Ruta insegura: ' + repr(name))
    for c in p.parts:
        if c.endswith((' ', '.')) or re.search(r'[<>"|?*\x00-\x1f]', c):
            raise ValueError('Nonportable entry path / Ruta no portable: ' + repr(name))
        if re.fullmatch(r'(?i:con|prn|aux|nul|com[1-9]|lpt[1-9])(?:\..*)?', c):
            raise ValueError('Reserved Windows path / Nombre reservado: ' + repr(name))
    return Path(*p.parts)


def extract(raw, output, provenance):
    metadata, entries = parse_container(raw)
    # Flag 1 entries observed in this build are wildcard index lists, not files.
    # Give them neutral physical names and retain original names in the manifest.
    seen = set()
    for i, entry in enumerate(entries):
        if entry['flag'] == 1:
            rel = Path('lists') / ('entry-%04d.bin' % i)
        else:
            try:
                rel = Path('files') / safe_relative(entry['name'])
            except ValueError:
                # SUGAR may export virtual source paths such as ../../libs/foo.lua.
                # Never resolve them against the host filesystem.
                suffix = PurePosixPath(entry['name']).suffix
                if not re.fullmatch(r'\.[A-Za-z0-9]{1,12}', suffix):
                    suffix = '.bin'
                rel = Path('virtual') / ('entry-%04d' % i + suffix)
        key = rel.as_posix().casefold()
        if key in seen:
            entry['duplicate_path'] = rel.as_posix()
            rel = Path('duplicates') / ('entry-%04d' % i + rel.suffix)
            key = rel.as_posix().casefold()
        seen.add(key)
        entry['output'] = rel.as_posix()
    output.mkdir(parents=True, exist_ok=False)
    for entry in entries:
        a = entry['offset']
        stored = raw[a:a+entry['stored_bytes']]
        data = transform(stored)
        target = output / entry['output']
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
        entry['bytes'] = len(data)
        entry['sha256'] = hashlib.sha256(data).hexdigest()
    result = {'format': 'SUGAR even-byte XOR + big-endian size + zlib',
              'provenance': provenance, 'metadata': metadata,
              'container_bytes': len(raw), 'container_sha256': hashlib.sha256(raw).hexdigest(),
              'entries': entries}
    (output/'manifest.json').write_text(json.dumps(result, indent=2, ensure_ascii=False)+'\n', encoding='utf-8')
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input', type=Path, help='Owned data.sgr / data.sgr de tu copia')
    parser.add_argument('output', type=Path, help='New private output directory / Directorio privado nuevo')
    parser.add_argument('--decoded-container', action='store_true', help='Input is an already decoded container / Entrada ya descomprimida')
    parser.add_argument('--max-mib', type=int, default=512, help='Input and decoded size limit / Límite de tamaño (MiB)')
    args = parser.parse_args()
    if args.output.exists():
        parser.error('Output already exists / La salida ya existe')
    limit = args.max_mib * 1024 * 1024
    if limit <= 0 or not 0 < args.input.stat().st_size <= limit:
        parser.error('Input size exceeds limit / Entrada fuera de límite')
    blob = args.input.read_bytes()
    source_hash = hashlib.sha256(blob).hexdigest()
    print('Reading container / Leyendo contenedor...', flush=True)
    raw = blob if args.decoded_container else unpack(blob, limit)
    print('Extracting entries / Extrayendo entradas...', flush=True)
    result = extract(raw, args.output, {'source_name': args.input.name, 'sha256': source_hash,
                     'input_kind': 'decoded container' if args.decoded_container else 'data.sgr'})
    print(json.dumps({'metadata': result['metadata'], 'entries': len(result['entries']),
                      'output': str(args.output.resolve())}, ensure_ascii=False, indent=2))


if __name__ == '__main__':
    main()
