"""Inspect owned Android packages without executing or extracting game code."""
import argparse
import hashlib
import json
import struct
import zipfile
from pathlib import Path

def inspect(path):
    result = {"file": path.name, "sha256": hashlib.file_digest(path.open("rb"), "sha256").hexdigest(), "libraries": [], "asset_count": 0}
    with zipfile.ZipFile(path) as archive:
        names = archive.namelist()
        result["asset_count"] = sum(n.startswith("assets/") and not n.endswith("/") for n in names)
        result["engine_markers"] = [n for n in names if any(s in n.lower() for s in ("unity", "libgodot", "liblove", "libyoyo", "sugar", "main.lua", "game.love"))]
        for name in names:
            if name.startswith("lib/") and name.endswith(".so"):
                with archive.open(name) as library:
                    header = library.read(64)
                valid = header[:4] == b"\x7fELF" and len(header) >= 20 and header[5] in (1, 2)
                machine = struct.unpack("<H" if header[5] == 1 else ">H", header[18:20])[0] if valid else None
                result["libraries"].append({"path": name, "elf_bits": header[4] * 32 if valid and header[4] in (1, 2) else None, "machine": machine, "arm32_candidate": valid and header[4] == 1 and machine == 40})
        result["arm32_present"] = any(lib["arm32_candidate"] for lib in result["libraries"])
        result["assessment"] = "ARM32 candidate: imports, graphics, JNI and entry point still need analysis" if result["arm32_present"] else "No ARM32 native library found in this package; check architecture splits"
    return result

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("apk", type=Path, nargs="+")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    report = json.dumps([inspect(path) for path in args.apk], indent=2)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(report + "\n", encoding="utf-8")
    print(report)
