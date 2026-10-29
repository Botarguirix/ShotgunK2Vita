"""Generate a private build-tree header: ordered, anchored code.lua replacements.

The runtime applies them while loading code.lua, so the original game file is
never modified. Each hunk is "old" (changed lines plus one line of context on
each side) and "new"; the runtime searches each "old" from the end of the
previous replacement, so hunks apply in file order.

Usage: gen_code_patches.py ORIGINAL.lua OPTIMIZED.lua TARGET.lua OUT.h
TARGET is the game file the patches must fit (e.g. the Steam code.lua); the
script refuses to write the header unless every hunk applies there.
"""
import argparse
import difflib
from pathlib import Path
import sys


def lines(path):
    return open(path, 'rb').read().decode('utf-8').replace('\r\n', '\n').split('\n')


def hunks(pre, opt):
    sm = difflib.SequenceMatcher(None, pre, opt, autojunk=False)
    # Merge changes whose one-line contexts touch, so hunks never overlap.
    groups = []
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == 'equal':
            continue
        if groups and i1 - 1 <= groups[-1][1] + 1:
            groups[-1][1], groups[-1][3] = i2, j2
        else:
            groups.append([i1, i2, j1, j2])
    out = []
    for i1, i2, j1, j2 in groups:
        a, b = max(0, i1 - 1), min(len(pre), i2 + 1)
        old = pre[a:b]
        new = pre[a:i1] + opt[j1:j2] + pre[i2:b]
        out.append(('\n'.join(old), '\n'.join(new)))
    return out


def apply(text, patches):
    cursor = 0
    for n, (old, new) in enumerate(patches):
        at = text.find(old, cursor)
        if at < 0:
            raise SystemExit(f'hunk {n} does not apply: {old[:80]!r}')
        text = text[:at] + new + text[at + len(old):]
        cursor = at + len(new)
    return text


def c_string(s):
    s = s.replace('\\', '\\\\').replace('"', '\\"').replace('\t', '\\t').replace('\n', '\\n"\n "')
    return '"' + s + '"'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--empty', action='store_true', help='Generate zero patches without game files')
    parser.add_argument('paths', nargs='+')
    args = parser.parse_args()
    if args.empty:
        if len(args.paths) != 1: parser.error('--empty requires only an output path')
        patches, out = [], args.paths[0]
    else:
        if len(args.paths) != 4: parser.error('Expected ORIGINAL OPTIMIZED TARGET OUTPUT')
        pre, opt, target, out = args.paths
        patches = hunks(lines(pre), lines(opt))
        apply('\n'.join(lines(target)), patches)
    target = Path(out)
    target.parent.mkdir(parents=True, exist_ok=True)
    with target.open('w', encoding='utf-8', newline='\n') as f:
        f.write('/* Private generated header; never commit or distribute it. */\n')
        f.write('#define SG_CODE_PATCH_COUNT %d\n' % len(patches))
        f.write('static const char *const sg_code_patches[][2]={\n')
        for old, new in patches:
            f.write(' {' + c_string(old) + ',\n  ' + c_string(new) + '},\n')
        if not patches: f.write(' {0,0},\n')
        f.write('};\n')
    print(f'{len(patches)} private patches generated')

if __name__ == '__main__':
    main()
