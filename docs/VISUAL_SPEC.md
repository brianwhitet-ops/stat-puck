# Visual spec — Instinct checkpoint (296×128)

Locked canvas: **296×128**, 1-bit, black ink on white paper, 1 px border.

Font: 1-bit DejaVu Sans Mono raster (Bitstream Vera / Arev — not Apple SF).
Labels 7×12 advance 8; values 10×16 advance 11. License: `firmware/fonts/dejavu-license.txt`.

Shared hole-card chrome (states 1–4):

| Band | Y | Content |
| --- | --- | --- |
| Header | 6 | `HOLE` + medium hole# · center state tag · `PAR` + medium par |
| Rule | 23 | x 10–285 |
| Strokes | 30 | `STROKES` + boxed value at x=92 |
| Putts | 52 | `PUTTS` + boxed value at x=92 |
| Rule | 75 | |
| Flags | 82 | FWY / drive chips left; **GIR Y/N overlay right** (never focused) |
| Rule | 103 | |
| Footer | 110 | this-hole vs-par (`+1 VS PAR`), or `LOCK HOLE n` |

Active field = filled invert box (white on black). Idle values are unboxed (concept scorecard).

## 1. DEFAULT_HOLE

Fixture: hole 7, par 4, strokes 5, putts 2, FWY L, GIR N (derived 5−2 ≰ 2), footer running vs-par.

No invert on values. Tag `HOLE`.

## 2. STROKE_EDIT

Same fixture. Strokes box inverted. Tag `STROKE`.

## 3. PUTTS_INPUT

Same fixture. Putts box inverted. Tag `PUTTS`.

## 4. END_HOLE_CONFIRM

Same fixture, par 4. Drive chips `L` `FWY` `R` with **L** inverted. Tag `LOCK`. Footer `LOCK HOLE 7 ?`.

Par 3 variant (extra frame `04b`): `DRIVE` + inverted `N/A` + `PAR 3`. No L/FWY/R chips.

## 5. ROUND_COMPLETE_SYNC

Full 18 locked. Header `ROUND COMPLETE` / `SYNC`. Count `18 / 18 HOLES` + vs-par. `BLE ADVERTISE` + inverted `ON`. Footer `PULL CARD` / `NO MID-ROUND`.

**Advertise only in this state.**

## 6. DERIVED_STATS

Header `DERIVED` / `STATS`. Rows GIR hits/holes, FWY hits/holes (par 3 excluded), PUTTS total. Footer `GIR FROM SCORE-PUTTS VS PAR`. Never shown as a play prompt.

Goldens: `firmware/testdata/golden/01_*.png` … `06_*.png`.
