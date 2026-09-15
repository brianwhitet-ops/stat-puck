#!/usr/bin/env python3
"""Pixel-compare 296x128 1-bit PNGs against locked goldens."""

from __future__ import annotations

import struct
import sys
import zlib
from pathlib import Path


STATES = [
    "01_default_hole",
    "02_stroke_edit",
    "03_putts_input",
    "04_end_hole_confirm",
    "05_round_complete_sync",
    "06_derived_stats",
]


def read_png_1bit(path: Path) -> tuple[int, int, bytes]:
    data = path.read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise SystemExit(f"{path}: not a PNG")
    pos = 8
    width = height = None
    idat = bytearray()
    while pos < len(data):
        (length,) = struct.unpack(">I", data[pos : pos + 4])
        ctype = data[pos + 4 : pos + 8]
        chunk = data[pos + 8 : pos + 8 + length]
        pos += 12 + length
        if ctype == b"IHDR":
            width, height, bit_depth, color_type = struct.unpack(">IIBB", chunk[:10])
            if bit_depth != 1 or color_type != 0:
                raise SystemExit(f"{path}: expected 1-bit greyscale, got {bit_depth}/{color_type}")
        elif ctype == b"IDAT":
            idat.extend(chunk)
        elif ctype == b"IEND":
            break
    if width is None:
        raise SystemExit(f"{path}: no IHDR")
    raw = zlib.decompress(bytes(idat))
    row_bytes = (width + 7) // 8
    pixels = bytearray()
    for y in range(height):
        start = y * (1 + row_bytes)
        if raw[start] != 0:
            raise SystemExit(f"{path}: unsupported filter {raw[start]}")
        pixels.extend(raw[start + 1 : start + 1 + row_bytes])
    return width, height, bytes(pixels)


def main() -> int:
    got_dir = Path(sys.argv[1] if len(sys.argv) > 1 else "firmware/testdata/out")
    gold_dir = Path(sys.argv[2] if len(sys.argv) > 2 else "firmware/testdata/golden")
    fail = 0
    for name in STATES:
        got_p = got_dir / f"{name}.png"
        gold_p = gold_dir / f"{name}.png"
        if not got_p.exists():
            print(f"FAIL missing {got_p}")
            fail += 1
            continue
        if not gold_p.exists():
            print(f"FAIL missing golden {gold_p}")
            fail += 1
            continue
        gw, gh, gp = read_png_1bit(got_p)
        fw, fh, fp = read_png_1bit(gold_p)
        if (gw, gh) != (296, 128) or (fw, fh) != (296, 128):
            print(f"FAIL {name} size got={gw}x{gh} gold={fw}x{fh}")
            fail += 1
            continue
        if gp != fp:
            diffs = sum(1 for a, b in zip(gp, fp) if a != b)
            print(f"FAIL {name} {diffs} packed-bytes differ")
            fail += 1
            continue
        print(f"ok  {name}  296x128")
    if fail:
        print(f"{fail} visual mismatches")
        return 1
    print("visual goldens ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
