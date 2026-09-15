# Slab firmware — visual-parity checkpoint

Instinct **REDO** visual layer on PR #1. State machine unchanged. Host/emulator output must pixel-match Instinct’s native 296×128 goldens.

Hardware target: Seeed **XIAO nRF52840** + Waveshare **2.9" B/W** SPI (296×128). On-device photos are a **later** gate.

## Instinct goldens (source of truth)

`design/instinct-firmware-goldens-pr1/`

| File | SHA256 (see `SHA256SUMS.txt`) |
| --- | --- |
| `01-default-hole.png` | 3fff64b5… |
| `02-stroke-edit.png` | b27d6646… |
| `03-putts-input.png` | c8520fdd… |
| `04-end-hole-confirm.png` | c128fded… |
| `05-round-complete-sync.png` | e925861c… |
| `06-derived-stats.png` | 32436720… |
| `slab-screen-states-contact-sheet.png` | da44600d… |

Copies of the six frames also live in `firmware/testdata/golden/`.

## Panel / driver lock (unchanged)

| Item | Choice |
| --- | --- |
| Module | Waveshare 2.9" e-Paper Module **V2/V3** B/W |
| Controller | **SSD1680** (GDEM029T94 / GDEY029T94) |
| Driver | **GxEPD2_290_T94_V2** |
| Full / partial | **~3 s** / **~0.3 s typ · 0.6 s listed** |

Partial on value edits; full on state change / hole lock / every 8th partial.

## Interaction lock (unchanged)

Per hole: **strokes → putts → done**. No dedicated GIR prompt. GIR is derived only.

| State | Role |
| --- | --- |
| `DEFAULT_HOLE` | Big current score (defaults strokes=par, putts=2). No FWY/GIR chrome. |
| `STROKE_EDIT` | Dial / +/− strokes · `PRESS DIAL TO KEEP` |
| `PUTTS_INPUT` | Dial / +/− putts · `PRESS DIAL TO KEEP` |
| `END_HOLE_CONFIRM` | `DONE`. Drive **LEFT / FAIRWAY / RIGHT** on par 4/5; no drive on par 3 |
| `ROUND_COMPLETE_SYNC` | **Only** BLE advertise gate · `READY TO SYNC` / `OPEN SLAB ON PHONE` |
| `DERIVED_STATS` | After the round. GIR + putting computed from score + putts |

`PUTT` pad is ignored (cannot skip). `MODE` = back / cycle drive / toggle stats. `NEXT` or dial-click advances.

## Host visual tests

```bash
make -C firmware/host visual
```

That command:

1. Verifies `SHA256SUMS.txt` against the Instinct pack
2. Runs state-machine tests
3. Renders six host PNGs from the dynamic composer (fixture/demo values)
4. Pixel-diffs them against the six goldens (must be **0** differing pixels)

Re-bake C frame blobs after replacing goldens:

```bash
python3 firmware/tools/bake_instinct_frames.py
```

## Device 1-bit panel path (non-host)

`slab_ui_render` copies Instinct chrome for the UI state, then **composes**
the packed 296×128 1-bit buffer from live round state (strokes, putts, hole,
drive, totals, copy). It does not blit a fixed PNG keyed only by enum.

When live fields equal the values those goldens encode, template pixels are
left alone (reproduces the thresholded goldens). Any other value is stamped
into the field box — `test_device_1bit` asserts those packed buffers differ.

PlatformIO (`xiaoble`) compiles `slab_frames_1bit.c` and **excludes**
`slab_frames.c` (zlib RGBA, host/emulator only).

The same non-host objects are built by `make -C firmware/host visual`
as `test_device_1bit` (`-DSLAB_HOST=0`).

```bash
pio run -e xiaoble -d firmware
```

On-device photos are a later gate.
