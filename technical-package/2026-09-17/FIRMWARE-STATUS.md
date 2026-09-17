# Firmware status (truth matrix)

**Source of truth:** PR #1 branch `cursor/slab-visual-parity-d2a5`  
**Commit:** `e7f7ad79c885627616819ad08114a704024569f7`  
**PR:** https://github.com/brianwhitet-ops/stat-puck/pull/1  
**`main` firmware:** `firmware/PINMAP.md` only — do not treat `main` as the firmware tree.

This matrix is grounded in that tree. **Host renders and `pio run` success are not hardware proof.**

## Explicit non-claims

| Claim someone might infer | Truth |
| --- | --- |
| Device boots a real inactive / start / course-card flow | **NOT IMPLEMENTED.** `slab_round_init` sets course `"LOCAL EXAMPLE"` / tees `"BLUE"` / fixed UUID. `setup()` loads LittleFS or that demo. |
| BLE / course-card / round transfer is end-to-end | **NOT E2E.** Advertise is a Serial stub. `slab_ble_build_payload` writes a JSON string in RAM. No GATT, no phone, no ACK. |
| Real Waveshare panel is proven | **No real-panel evidence** in-repo. |
| Power / battery life is proven | **No measured currents.** |
| Power-loss / queued-round integrity is proven | **No evidence.** Persist write/read hooks exist. |
| Provisioning / unique device ID | `SLAB_DEVICE_ID` is the literal `"SPK-0001"`. |
| OTA / BLE DFU works because nRF52840 can | **NOT IMPLEMENTED** on PR #1. See `BLE-DFU.md`. |
| PR #3 implements DFU | **NOT IMPLEMENTED.** Visual shell + contract helpers only. |

## Status key

| Tag | Meaning |
| --- | --- |
| **I** | Implemented in PR #1 source |
| **H** | Exercised by host tests (`make -C firmware/host visual` claimed in PR body) |
| **X** | XIAO **build** claimed (`pio run -e xiaoble -d firmware` SUCCESS in PR body at `e7f7ad7`) |
| **P** | Real-panel / on-hardware behavior evidenced in-repo |
| **N** | Not implemented |

`H`/`X` never upgrade a row to `P`.

## Matrix

| Item | I | H | X | P | N | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| `LOCAL EXAMPLE` demo boot | Y | Y | Y* | — | | `slab_round_init`. *X = compiles into `setup()`, not a flashed-device log |
| Instinct 296×128 chrome + live field compose | Y | Y | Y* | — | | `slab_ui_render` stamps strokes/putts/hole/drive/totals; goldens 0-diff at fixture values |
| Device packed 1-bit buffer changes with live fields | Y | Y | Y* | — | | `test_device_1bit` (`SLAB_HOST=0`) |
| Sequential lock strokes → putts → done | Y | Y | Y* | — | | `test_state.c` |
| No GIR prompt; GIR derived | Y | Y | Y* | — | | `slab_hole_gir` |
| Drive L/fairway/R on par 4/5 only | Y | Y | Y* | — | | PUTT ignored |
| BLE advertise **gate** (legal only in `ROUND_COMPLETE_SYNC`) | Y | Y | Y* | — | | `slab_ble_should_advertise` |
| Bluefruit / SoftDevice advertising | — | — | — | — | **N** | `main.cpp` Serial print stub |
| GATT CourseCard / RoundData / SyncControl / DeviceInfo | — | — | — | — | **N** | Named in stale `docs/PHONE_APP.md` only |
| Course-card download / tee pars from phone | — | — | — | — | **N** | Hardcoded `kDefaultPars` / `kYards` |
| Inactive / start / Quick Start UI | — | — | — | — | **N** | |
| “ROUND SAVED ON SLAB” before sync copy | — | — | — | — | **N** | Sync screen copy is Instinct golden, not a save-state machine |
| Multi-round queue | — | — | — | — | **N** | One `slab_round_t` |
| ACK / idempotent import on device | — | — | — | — | **N** | `synced` flag exists; unused E2E |
| Host persist file | Y | Y | — | — | | `nvram.bin` |
| Device LittleFS `/slab_round.bin` | Y | — | Y* | — | | Weak hooks implemented in `main.cpp`; **unproven on metal** |
| Power-loss mid-hole / mid-write | — | — | — | — | **N** | No test |
| SSD1680 / GxEPD2 refresh on **real glass** | Y* | — | Y* | — | **N** as evidence | Code path `push_fb` exists; no photo/log |
| Button debounce 25 ms on hardware | Y | Y | Y* | — | | Host unit tests; no scope trace |
| Measured system current | — | — | — | — | **N** | |
| Provisioning / serial / keys | — | — | — | — | **N** | |
| **OTA / BLE DFU / bootloader** | — | — | — | — | **N** | PR #1 has **zero** DFU sources |
| Signed image / rollback / dual-bank | — | — | — | — | **N** | |

## Tree map (PR #1 @ `e7f7ad79…`)

| Path | Role |
| --- | --- |
| `firmware/src/main.cpp` | Arduino `setup`/`loop`; GxEPD2; LittleFS; BLE stub |
| `firmware/src/slab_state.c` | SM, defaults, derived GIR/FWY, fixtures |
| `firmware/src/slab_ui.c` + `slab_frames*.c` | Compose from Instinct templates |
| `firmware/src/slab_ble.c` | Gate + JSON builder (`stat-puck.round.v1`) |
| `firmware/src/slab_persist.c` | Host file / device weak hooks |
| `firmware/src/slab_input.c` | Debounce / events |
| `firmware/src/slab_epd.c` | Partial-before-full policy |
| `firmware/platformio.ini` | `maxgerhardt/platform-nordicnrf52` + `xiaoble_adafruit` |
| `firmware/host/Makefile` | `visual` target |
| `design/instinct-firmware-goldens-pr1/` | Locked PNGs + SHA256 |
| `firmware/README.md` | **Empty file (0 bytes)** |

## Test commands cited by PR #1 (not re-run on this branch)

```bash
make -C firmware/host visual
pio run -e xiaoble -d firmware
```

This package branch does **not** contain the PR #1 tree. Point reviewers at the commit above. Do not copy goldens here unless asked (binary, already on PR #1).

## Relation to production DFU

Adding Nordic examples later does not change this matrix until a signed, brick-safe, app-driven path exists and is tested. Mark every DFU row **N** until then.
