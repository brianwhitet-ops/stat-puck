# Visual spec — Instinct REDO (296×128)

**Source of truth:** `design/instinct-firmware-goldens-pr1/`  
Hashes: `design/instinct-firmware-goldens-pr1/SHA256SUMS.txt`  
Note: `design/instinct-firmware-goldens-pr1/REDO-NOTE.md`

These native 296×128 RGBA frames replace the prior host 1-bit scorecard. No layout reinterpretation.

| File | State | Locked copy |
| --- | --- | --- |
| `01-default-hole.png` | `DEFAULT_HOLE` | Big score (par). `HOLE 07` · `PAR 4 · 412 YD`. `E · THROUGH 6`. No FWY/GIR. |
| `02-stroke-edit.png` | `STROKE_EDIT` | `STROKES` · − 4 + · `PRESS DIAL TO KEEP` |
| `03-putts-input.png` | `PUTTS_INPUT` | `PUTTS` · − 2 + · `PRESS DIAL TO KEEP` |
| `04-end-hole-confirm.png` | `END_HOLE_CONFIRM` | `DONE` · strokes/putts · drive LEFT / **FAIRWAY** / RIGHT · `PRESS DIAL` |
| `05-round-complete-sync.png` | `ROUND_COMPLETE_SYNC` | `ROUND COMPLETE` · 74 · +2 · `READY TO SYNC` · `OPEN SLAB ON PHONE` |
| `06-derived-stats.png` | `DERIVED_STATS` | `ROUND STATS` · GIR 11/18 61% · putting 31 / 1.72 per hole · `COMPUTED FROM SCORE+PUTTS` |
| `slab-screen-states-contact-sheet.png` | (sheet) | Instinct 2×3 |

Host `slab_ui_render` starts from the locked frame (zlib-baked in `firmware/src/slab_frames.c`) and composes live fields from `slab_round_t`. With the fixture/demo values these goldens encode, pixel-diff must be zero vs these PNGs. Other live values change those field pixels.

Behavior (unchanged): strokes → putts → done; no GIR prompt; drive L/fairway/R on par 4/5 only; BLE advertise only in `ROUND_COMPLETE_SYNC`.
