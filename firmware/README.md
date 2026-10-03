# Slab firmware workstream

This branch builds on firmware PR #1. The companion app change builds on PR #3. Neither existing branch needs to be merged to review these changes, and no main branch merge or public website deployment is part of this work.

Read `../protocol/README.md` for the v2 contract and integration boundary. Read `../docs/SOFTWARE-HANDOFF-2026-09-18.md` for remaining vendor work.

## Implemented here

- On the built-in par-72 card, strokes open at par and drive opens on Fairway (+ Left, − Right, opposite returns to Fairway). Unknown par still stays null until entered.
- Hold MODE (~2s) enters System OFF on every screen. MODE wakes the chip.
- MODE on an untouched hole reopens only the hole just finished.
- Missing data stays null on the wire and stays out of statistics.
- Explicit 9/18 Quick Start and end-as-nine at the turn.
- Unique round ids from the XIAO silicon identity and a persisted sequence; golden fixtures are explicitly bench-only.
- Ten-round FIFO, queue-full backpressure, non-destructive chunk reads, identity/checksum-bound ACK retirement.
- Versioned two-slot save journal for active and completed rounds. UI/save failures stop progress instead of showing a false success.
- Protocol v2 C encoder with bounded output and a matching TypeScript implementation.
- Post-round BLE advertising as `Slab` (Adafruit Bluefruit on `xiaoble` / `xiaoble_adafruit`). GATT read of one completed round, page and offset windows, and a meta-JSON ACK. See `BLE_SYNC.md`.
- Empty storage boots into a preloaded completed demo round so a phone can sync before 18 holes are played. Serial `seed` appends another.

## Verification

```sh
make -C firmware/host test
make -C firmware/host visual
make -C firmware/host fixtures
python -m platformio run -e xiaoble -d firmware
python3 firmware/tools/provision/check_no_autoformat.py
python -m platformio run -e xiaoble_provision -d firmware
```

The scoring UF2 is `firmware/dist/slab-xiaoble.uf2` (env `xiaoble`, board `xiaoble_adafruit`). Flash steps are in `BLE_SYNC.md`.

Host tests run state, packed-device framebuffer, protocol/queue and byte-by-byte interrupted-write checks. `visual` compares six original fixtures without overwriting their goldens. Additional renders show zero-start and unentered-putt behavior. Tests are software evidence only.

## Still unimplemented or unverified

Phone/native BLE bridge; course-card download; signed DFU/rollback/provisioning; production board support; measured current and sleep optimization; real-panel refresh, button timing, RF range, environmental tests and physical power-cut testing. The GATT image is a sync-screen read path plus one ACK write (`BLE_SYNC.md`); pairing, transfer sessions, and phone-side durability are still open. The existing blocking display driver pauses button polling during refresh and requires hardware timing work. No production battery-life claim is made. The panel driver remains stock `GxEPD2_290_T94_V2`.

Provision the filesystem separately on fresh boards. Normal startup only mounts it; it never invokes the SDK auto-format path. A mount failure shows a storage error and preserves the medium for service. The bench erase image is `firmware/tools/provision/` (`pio run -e xiaoble_provision`). PlatformIO 6.2 ignores a per-environment `src_dir`, so that env's pre-script selects the sketch. It formats only after the operator types the serial phrase in that directory's README. A blank mount does not contain the journal; the scoring firmware's existing empty-store path creates it on first boot. That image has not been run on a XIAO.

The selected development platform is the existing XIAO Adafruit/Arduino bench framework. This work does not select the production MCU, radio stack or bootloader. Large frame/store/journal workspaces are static to avoid overflowing the small Arduino loop-task stack. Integrators must serialize access and measure stack high-water and flash wear on hardware.
