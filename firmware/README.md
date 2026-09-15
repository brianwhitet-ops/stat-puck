# Slab firmware — visual-parity checkpoint

First locked UI/state machine for the cart e-ink scorer. **Stop here for Instinct pixel review.** Do not continue past this checkpoint.

Hardware target: Seeed **XIAO nRF52840** + Waveshare **2.9" B/W** SPI (296×128).

## Panel / driver lock

| Item | Choice |
| --- | --- |
| Module | Waveshare 2.9" e-Paper Module **V2/V3** B/W (SKU-class 296×128) |
| Panel family | Good Display **GDEM029T94 / GDEY029T94** |
| Controller | **SSD1680** |
| Driver | **GxEPD2_290_T94_V2** (partial waveform written to registers; V2 OTP lacks it) |
| V1 only | IL3820/SSD1608 via `GxEPD2_290` — **not** the P1 lock |

Refresh times (Waveshare product / wiki):

- Full: **~3 s** (flicker; clears ghosting)
- Partial: **~0.3 s typical / 0.6 s listed**

Policy in `slab_epd.c` / `include/slab_epd.h`:

- **Partial** for in-place stroke/putt value edits
- **Full** on every UI state change, hole lock, round-complete, and every 8th partial
- Wait module **BUSY = high**, timeout 5 s
- Hibernate panel between refreshes

## Interaction lock

Per hole: **strokes → putts → done**. No dedicated GIR prompt.

| State | Role |
| --- | --- |
| `DEFAULT_HOLE` | Resting hole card. Defaults strokes=par, putts=2 |
| `STROKE_EDIT` | +/− / dial change strokes |
| `PUTTS_INPUT` | +/− / dial change putts |
| `END_HOLE_CONFIRM` | Drive **L / FWY / R** on par 4/5 only; par 3 is `DRIVE N/A` |
| `ROUND_COMPLETE_SYNC` | **Only** legal BLE advertise gate |
| `DERIVED_STATS` | Overlay after the round. GIR = `(strokes − putts) ≤ (par − 2)` |

`PUTT` pad is wired but **ignored** so it cannot skip the sequence. `MODE` = back (or cycle drive on confirm, or toggle stats after 18). `NEXT` advances.

BLE advertise is `false` in every mid-round state. Payload stub matches `docs/SYNC_PAYLOAD.md`. No GHIN fields.

Hole card + UI persist across reset via host file / on-device LittleFS (`/slab_round.bin`).

Pads: 25 ms debounce (B3W bounce ≤ 5 ms). Optional quadrature dial uses the same +/− path (4 pulses / detent).

Font: 1-bit **DejaVu Sans Mono** raster (Bitstream Vera / Arev, not Apple SF). See `firmware/fonts/dejavu-license.txt`.

## Host visual tests (no hardware)

From repo root:

```bash
python3 firmware/tools/make_font.py   # only if regenerating slab_font.c
make -C firmware/host goldens         # rewrite locked 296×128 PNGs
make -C firmware/host visual          # state tests + pixel-compare goldens
```

Or:

```bash
cd firmware/host && make visual
```

Six locked frames (296×128, 1-bit PNG):

- `firmware/testdata/golden/01_default_hole.png`
- `firmware/testdata/golden/02_stroke_edit.png`
- `firmware/testdata/golden/03_putts_input.png`
- `firmware/testdata/golden/04_end_hole_confirm.png` (par 4, drive L/FWY/R)
- `firmware/testdata/golden/05_round_complete_sync.png`
- `firmware/testdata/golden/06_derived_stats.png`

Extra (not one of the six): `firmware/testdata/extra/04b_end_hole_confirm_par3.png` — same confirm state, no drive chips.

Layout coordinates: `docs/VISUAL_SPEC.md`.

## Bench / device build

Host bench binary: `firmware/host/bench_render` (gcc + zlib).

Device (when the XIAO is on the bench):

```bash
pio run -e xiaoble -d firmware
pio run -e xiaoble -d firmware -t upload
```

`platformio.ini` pulls GxEPD2. Pin map: `firmware/PINMAP.md`.

## Checkpoint status

Ready for Instinct 296×128 pixel review. No further UI states or redesign in this PR.
