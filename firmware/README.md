# Slab firmware workstream

This branch builds on firmware PR #1. The companion app change builds on PR #3. Neither existing branch needs to be merged to review these changes, and no main branch merge or public website deployment is part of this work.

Read `../protocol/README.md` for the v2 contract and integration boundary. Read `../docs/SOFTWARE-HANDOFF-2026-09-18.md` for remaining vendor work.

## Implemented here

- Explicit zero-start capture flags; no par/two-putt/fairway prefills.
- Missing data stays null on the wire and stays out of statistics.
- Explicit 9/18 Quick Start and end-as-nine at the turn.
- Unique round ids from the XIAO silicon identity and a persisted sequence; golden fixtures are explicitly bench-only.
- Ten-round FIFO, queue-full backpressure, non-destructive chunk reads, identity/checksum-bound ACK retirement.
- Versioned two-slot save journal for active and completed rounds. UI/save failures stop progress instead of showing a false success.
- Protocol v2 C encoder with bounded output and a matching TypeScript implementation.

## Verification

```sh
make -C firmware/host test
make -C firmware/host visual
make -C firmware/host fixtures
python -m platformio run -e xiaoble -d firmware
python3 firmware/tools/provision/check_no_autoformat.py
python -m platformio run -e xiaoble_provision -d firmware
```

Host tests run state, packed-device framebuffer, protocol/queue and byte-by-byte interrupted-write checks. `visual` compares six original fixtures without overwriting their goldens. Additional renders show zero-start and unentered-putt behavior. Tests are software evidence only.

## Still unimplemented or unverified

Real GATT advertising/transfer and ACK callbacks; phone/native BLE bridge; course-card download; signed DFU/rollback/provisioning; production board support; measured current and sleep optimization; real-panel refresh, button timing, RF range, environmental tests and physical power-cut testing. The existing blocking display driver pauses button polling during refresh and requires hardware timing work. No production battery-life claim is made.

Provision the filesystem separately on fresh boards. Normal startup only mounts it; it never invokes the SDK auto-format path. A mount failure shows a storage error and preserves the medium for service. The bench erase image is `firmware/tools/provision/` (`pio run -e xiaoble_provision`), and it formats only after the operator types the serial phrase in that directory's README. A blank mount does not contain the journal; the scoring firmware's existing empty-store path creates it on first boot. That image has not been run on a XIAO.

The selected development platform is the existing XIAO Adafruit/Arduino bench framework. This work does not select the production MCU, radio stack or bootloader. Large frame/store/journal workspaces are static to avoid overflowing the small Arduino loop-task stack. Integrators must serialize access and measure stack high-water and flash wear on hardware.
