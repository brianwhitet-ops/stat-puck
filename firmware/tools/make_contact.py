#!/usr/bin/env python3
"""2x3 contact sheet of the six locked 296x128 frames (nearest-neighbor 3x)."""

from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
GOLD = ROOT / "testdata" / "golden"
OUT = ROOT / "testdata" / "extra" / "contact_six_states_x3.png"
NAMES = [
    "01_default_hole",
    "02_stroke_edit",
    "03_putts_input",
    "04_end_hole_confirm",
    "05_round_complete_sync",
    "06_derived_stats",
]


def main() -> None:
    scale = 3
    gutter = 8
    imgs = []
    for name in NAMES:
        im = Image.open(GOLD / f"{name}.png").convert("L")
        imgs.append(im.resize((296 * scale, 128 * scale), Image.NEAREST))
    w, h = 296 * scale, 128 * scale
    sheet = Image.new("L", (w * 2 + gutter, h * 3 + gutter * 2), 180)
    for i, im in enumerate(imgs):
        sheet.paste(im, ((i % 2) * (w + gutter), (i // 2) * (h + gutter)))
    OUT.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(OUT)
    print(f"wrote {OUT} {sheet.size[0]}x{sheet.size[1]}")


if __name__ == "__main__":
    main()
