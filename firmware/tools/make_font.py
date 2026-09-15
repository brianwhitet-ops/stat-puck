#!/usr/bin/env python3
"""Bake DejaVu Sans Mono (Bitstream Vera / Arev, freely redistributable) to 1-bit C.

Not Apple SF. Source TTF: /usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf
"""

from __future__ import annotations

from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "src" / "slab_font.c"
TTF = Path("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf")

# Cell sizes locked for the 296x128 layout.
S_W, S_H = 7, 12
M_W, M_H = 10, 16
S_SIZE = 11
M_SIZE = 16


def render_glyph(ch: str, size: int, cell_w: int, cell_h: int) -> Image.Image:
    font = ImageFont.truetype(str(TTF), size)
    canvas = Image.new("L", (cell_w + 8, cell_h + 8), 255)
    draw = ImageDraw.Draw(canvas)
    # DejaVu has a few pixels of top bearing; park baseline so the cell fills.
    draw.text((0, -1), ch, font=font, fill=0)
    crop = canvas.crop((0, 0, cell_w, cell_h))
    bw = crop.point(lambda p: 0 if p < 160 else 255, mode="1")
    return bw


def rows_from_img(img: Image.Image) -> list[int]:
    w, h = img.size
    out: list[int] = []
    px = img.load()
    for y in range(h):
        if w <= 8:
            bits = 0
            for x in range(w):
                if px[x, y] == 0:
                    bits |= 0x80 >> x
            out.append(bits)
        else:
            b0 = b1 = 0
            for x in range(w):
                if px[x, y] != 0:
                    continue
                if x < 8:
                    b0 |= 0x80 >> x
                else:
                    b1 |= 0x80 >> (x - 8)
            out.extend([b0, b1])
    return out


def emit_table(name: str, size: int, cell_w: int, cell_h: int, stride: int) -> str:
    lines = [f"static const uint8_t {name}[{96 * stride}] = {{"]
    for code in range(32, 128):
        ch = chr(code)
        img = render_glyph(ch, size, cell_w, cell_h)
        data = rows_from_img(img)
        if len(data) != stride:
            raise SystemExit(f"{name} {ch!r} stride {len(data)} != {stride}")
        comment = ch if ch.isprintable() and ch not in "\\" else hex(code)
        hexes = ", ".join(f"0x{b:02X}" for b in data)
        lines.append(f"    {hexes}, /* {comment} */")
    lines.append("};")
    return "\n".join(lines)


def main() -> None:
    s_stride = S_H
    m_stride = M_H * 2
    s_table = emit_table("kFontS", S_SIZE, S_W, S_H, s_stride)
    m_table = emit_table("kFontM", M_SIZE, M_W, M_H, m_stride)
    mid = rows_from_img(render_glyph("·", S_SIZE, S_W, S_H))

    text = f"""/* SPDX-License-Identifier: MIT */
/*
 * Slab Mono cells — 1-bit raster of DejaVu Sans Mono (Bitstream Vera / Arev).
 * Not Apple SF. DejaVu license: firmware/fonts/dejavu-license.txt
 */
#include "slab_font.h"

{s_table}

{m_table}

static const uint8_t kMiddotS[{S_H}] = {{ {", ".join(f"0x{b:02X}" for b in mid)} }};

const uint8_t *slab_glyph_s(char c)
{{
    unsigned char u = (unsigned char)c;
    if (u == 0xB7) {{
        return kMiddotS;
    }}
    if (u < 32 || u > 127) {{
        return &kFontS[0];
    }}
    return &kFontS[(u - 32) * {s_stride}];
}}

const uint8_t *slab_glyph_m(char c)
{{
    unsigned char u = (unsigned char)c;
    if (u < 32 || u > 127) {{
        return &kFontM[0];
    }}
    return &kFontM[(u - 32) * {m_stride}];
}}
"""
    OUT.write_text(text)
    print(f"wrote {OUT}")


if __name__ == "__main__":
    main()
