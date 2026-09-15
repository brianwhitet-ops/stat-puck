#!/usr/bin/env python3
"""Pixel-diff host RGBA output against Instinct 296x128 goldens."""

from __future__ import annotations

import sys
from pathlib import Path

from PIL import Image

STATES = [
    "01-default-hole",
    "02-stroke-edit",
    "03-putts-input",
    "04-end-hole-confirm",
    "05-round-complete-sync",
    "06-derived-stats",
]


def pixels(path: Path) -> tuple[int, int, list[tuple[int, int, int, int]]]:
    im = Image.open(path).convert("RGBA")
    raw = im.tobytes()
    pix = [tuple(raw[i : i + 4]) for i in range(0, len(raw), 4)]
    return im.size[0], im.size[1], pix


def main() -> int:
    got_dir = Path(sys.argv[1] if len(sys.argv) > 1 else "firmware/testdata/out")
    gold_dir = Path(sys.argv[2] if len(sys.argv) > 2 else "design/instinct-firmware-goldens-pr1")
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
        gw, gh, gp = pixels(got_p)
        fw, fh, fp = pixels(gold_p)
        if (gw, gh) != (296, 128) or (fw, fh) != (296, 128):
            print(f"FAIL {name} size got={gw}x{gh} gold={fw}x{fh}")
            fail += 1
            continue
        diffs = sum(1 for a, b in zip(gp, fp) if a != b)
        if diffs:
            print(f"FAIL {name} {diffs} pixels differ")
            fail += 1
            continue
        print(f"ok  {name}  296x128  0 pixel diffs")
    if fail:
        print(f"{fail} visual mismatches")
        return 1
    print("visual goldens ok (pixel-identical)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
