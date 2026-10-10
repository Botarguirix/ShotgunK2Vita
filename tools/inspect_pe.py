"""Read PE headers, imports, exports and selected strings without executing code."""
import argparse
import hashlib
import json
import re
import struct
from pathlib import Path

def inspect(path):
    data = path.read_bytes()
    def u16(p): return struct.unpack_from('<H', data, p)[0]
    def u32(p): return struct.unpack_from('<I', data, p)[0]
    pe = u32(0x3c)
    if data[:2] != b'MZ' or data[pe:pe+4] != b'PE\0\0':
        raise ValueError('Not a PE file: ' + str(path))
    optional = pe + 24
    bits = 64 if u16(optional) == 0x20b else 32
    base = struct.unpack_from('<Q' if bits == 64 else '<I', data, optional + (24 if bits == 64 else 28))[0]
    table = optional + u16(pe+20)
    symbol_offset, symbol_count = u32(pe+12), u32(pe+16)
    string_table = symbol_offset + symbol_count*18
    coff_valid = bool(symbol_offset and string_table+4 <= len(data))
    def coff_name(raw):
        if raw[:4] == b'\0'*4 and string_table and string_table < len(data):
            pos = string_table + struct.unpack_from('<I', raw, 4)[0]
            return data[pos:data.index(b'\0',pos)].decode(errors='replace')
        return raw.rstrip(b'\0').decode(errors='replace')
    sections = []
    for i in range(u16(pe+6)):
        pos = table + i*40
        sections.append({'name': data[pos:pos+8].rstrip(b'\0').decode(errors='replace'), 'virtual_size': u32(pos+8), 'rva': u32(pos+12), 'raw_size': u32(pos+16), 'raw_offset': u32(pos+20), 'flags': hex(u32(pos+36))})
        if sections[-1]['name'].startswith('/') and coff_valid:
            start = string_table + int(sections[-1]['name'][1:])
            sections[-1]['name'] = data[start:data.index(b'\0',start)].decode(errors='replace')
    symbols = []
    i = 0
    while coff_valid and i < symbol_count:
        pos = symbol_offset+i*18
        name = coff_name(data[pos:pos+8])
        value, section, kind, storage, aux = struct.unpack_from('<IhHBB',data,pos+8)
        symbols.append({'name':name, 'value':hex(value), 'section':section, 'type':kind, 'storage':storage})
        i += 1+aux
    def offset(rva):
        for section in sections:
            if section['rva'] <= rva < section['rva'] + max(section['virtual_size'], section['raw_size']):
                return section['raw_offset'] + rva - section['rva']
        if rva < u32(optional+60): return rva
        raise ValueError('Unmapped RVA: ' + hex(rva))
    def string(pos): return data[pos:data.index(b'\0', pos)].decode(errors='replace')
    directories = optional + (112 if bits == 64 else 96)
    imports = {}
    imp_rva = u32(directories+8)
    if imp_rva:
        pos = offset(imp_rva)
        while any(data[pos:pos+20]):
            thunk = u32(pos) or u32(pos+16)
            dll = string(offset(u32(pos+12)))
            names = []
            pointer = offset(thunk)
            width = bits//8
            while True:
                value = struct.unpack_from('<Q' if bits == 64 else '<I', data, pointer)[0]
                if not value: break
                names.append('#'+str(value & 65535) if value & (1 << (bits-1)) else string(offset(value)+2))
                pointer += width
            imports[dll] = names
            pos += 20
    exports = []
    exp_rva = u32(directories)
    if exp_rva:
        pos = offset(exp_rva)
        count = u32(pos+24)
        if count:
            names = offset(u32(pos+32))
            exports = [string(offset(u32(names+i*4))) for i in range(count)]
    selected = []
    pattern = re.compile(rb'sugar|data\.sgr|shaderless|lua|\.pdb|SDL|newwin|newsrf|sgr|decrypt|inflate|uncompress', re.I)
    for match in re.finditer(rb'[\x20-\x7e]{5,}', data):
        if pattern.search(match.group()):
            raw = match.start()
            address = None
            for section in sections:
                if section['raw_offset'] <= raw < section['raw_offset']+section['raw_size']:
                    address = base+section['rva']+raw-section['raw_offset']
                    break
            selected.append({'offset': hex(raw), 'address': hex(address) if address else None, 'text': match.group().decode()[:500]})
    end = max(s['raw_offset']+s['raw_size'] for s in sections)
    return {'file':path.name, 'sha256':hashlib.sha256(data).hexdigest(), 'bits':bits, 'machine':hex(u16(pe+4)), 'image_base':hex(base), 'entry_address':hex(base+u32(optional+16)), 'sections':sections, 'imports':imports, 'exports':exports, 'symbols':symbols, 'selected_strings':selected, 'trailing_bytes':max(0,len(data)-end)}

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('files', type=Path, nargs='+')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    reports = [inspect(path) for path in args.files]
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(reports, indent=2), encoding='utf-8')
    for report in reports:
        print(report['file'], report['bits'], 'bits; entry', report['entry_address'], '; dependencies:', ', '.join(report['imports']))
