# Slab LittleFS factory provisioner

Separate bench image for a Seeed XIAO nRF52840. It formats the Adafruit internal LittleFS region only after the operator types an exact serial line. It is not the scoring firmware.

Scoring firmware stays fail-closed. `firmware/src/main.cpp` mounts with `InternalFS.Adafruit_LittleFS::begin()` and does not call `InternalFS.begin()` or `format()`. A failed mount sets the storage error and draws `STORAGE ERROR` / `NO DATA ERASED`. Nothing in that boot path erases the filesystem.

Build success is not silicon success. Brian’s XIAO and a data-capable USB-C cable were not available for this change. No serial log and no panel photo were captured from hardware.

## What this image erases

After the phrase below is received, the sketch page-erases the same internal-flash window the Seeed Adafruit core uses for LittleFS on nRF52840:

- start `0x000ED000`
- 7 pages × 4096 bytes = 28672 bytes
- end `0x000F4000`, which is `BOOTLOADER_ADDR` in that core’s `flash_nrf5x.c`

That window holds `/slab_store.0`, `/slab_store.1`, and a legacy `/slab_round.bin` if one was present. The sketch does not erase the SoftDevice, the application, or the bootloader. Those bounds are taken from framework commit `3ae21c3c6c544fed6e949f00986324dd95fb2b89` (`InternalFileSystem.cpp`, `NRF52840_XXAA`). They are not a measurement from this board.

The public `InternalFS.begin()` override in that core formats by itself when mount fails. This sketch never calls it. Erase is `flash_nrf5x_erase` on those seven pages, then `InternalFS.format()`, then the same mount-only `Adafruit_LittleFS::begin()` the scoring firmware uses.

## Warning copy

The sketch prints these lines on USB serial at 115200 before it will accept the phrase. The source check compares this block to `kWarning` in `main.cpp`.

<!-- warning-begin -->
```
======== SLAB FACTORY PROVISIONER ========
NOT SCORING FIRMWARE. NOT THE PRODUCTION BOOT PATH.
WARNING: THE NEXT CONFIRMED STEP ERASES INTERNAL LITTLEFS.
Region 0x000ED000 + 28672 bytes (7 pages of 4096).
This destroys /slab_store.0, /slab_store.1, and /slab_round.bin.
It does not erase the application, SoftDevice, or bootloader.
A USB-C data cable is required. A charge-only cable cannot confirm this step.
Reset now to leave flash UNCHANGED.
No erase happens until this exact line is received:
ERASE SLAB FS
==========================================
```
<!-- warning-end -->

The confirming line is exactly `ERASE SLAB FS` followed by Enter. Any other non-empty line prints `REFUSED` and does not erase. A blank Enter is ignored so the extra half of a CRLF is not a refusal. Reset the board to try again after `REFUSED`. There is no timeout that erases.

Result tokens:

- `REFUSED` — phrase was not exact. Flash was not erased.
- `PROVISION FAILED` — erase was started and did not finish cleanly. Do not flash scoring firmware and expect Quick Start. Run this image again until `PROVISION OK`.
- `PROVISION OK` — blank LittleFS mounted. `/slab_store.0`, `/slab_store.1`, and `/slab_round.bin` were absent. This image did not write the journal.

Until `PROVISION OK` or `PROVISION FAILED` is printed, do not disconnect USB and do not close the serial monitor. Closing the monitor can reset the native USB port mid-erase.

## Empty journal after a blank mount

This image leaves a mountable filesystem with no journal files. It does not write `/slab_store.0` or `/slab_store.1`.

Scoring `setup()` already handles that case. The mount-only call runs first. If it fails, the status is `SLAB_PERSIST_ERROR` and the panel shows `STORAGE ERROR` / `NO DATA ERASED` with no format. If it succeeds, `slab_persist_load_store` returns `SLAB_PERSIST_EMPTY` only when both slot reads are absent. A slot read is absent only when that slot file is missing and `/slab_round.bin` is also missing. An invalid slot, a size mismatch, or the legacy file is `SLAB_PERSIST_ERROR`, not empty, and startup does not reinitialize the store.

On `SLAB_PERSIST_EMPTY`, `setup()` calls `slab_store_init` (sequence 1, no active round, queue length 0) and `slab_persist_save_store`. That save is the normal journal write: generation 1, inactive slot `/slab_store.0`, flush, and read-back. If the save fails, the panel shows `STORAGE ERROR` and the scoring loop stops. It still does not format.

The host journal test drives the same load/save code with absent slots and checks that the first load is empty. It does not mount LittleFS and it does not touch nRF flash. The 2026-09-18 handoff recorded that test walking every byte of a 3328-byte snapshot. Re-run `make -C firmware/host test` and read the `journal:` line rather than assuming that size still matches.

The first scoring boot after `PROVISION OK` is what creates the empty store. The second scoring boot loads that store. With no round started, `g_start_screen` stays true, so the panel draws `QUICK START` and `0 SAVED / MODE TO VIEW`. That screen path is what a later photo needs to show. Scoring firmware does not print `QUICK START` or `STORAGE ERROR` on serial; those strings are drawn by `render_screen()`.

## Operator procedure

Stop at the first failed condition. Do not keep resetting a `STORAGE ERROR` board in the hope that scoring firmware will format it. It will not.

1. Install PlatformIO Core 6. The command used in this workspace is `python3 -m pip install --user platformio` (PlatformIO 6.2.0). Put `~/.local/bin` on `PATH`, or call `python3 -m platformio`.
2. Clone `https://github.com/brianwhitet-ops/stat-puck` and check out this provisioner branch. Do not commit onto `codex/slab-scoring-protocol` and do not merge to `main`.
3. Confirm the parent commit is `85092686dd7270c0e37f1512ddef3ccc1df30e02` (`git merge-base HEAD 85092686dd7270c0e37f1512ddef3ccc1df30e02`).
4. From the repo root, run the source guard:

   ```sh
   python3 firmware/tools/provision/check_no_autoformat.py
   ```

5. Build the provisioner and the scoring firmware as two different images:

   ```sh
   python3 -m platformio run -e xiaoble_provision -d firmware
   python3 -m platformio run -e xiaoble -d firmware
   ```

   `pio run -t upload` without `-e` can upload every env. Do not do that. Upload one env at a time.
6. Use a USB-C cable that carries data. This step is required and is unproven here: no XIAO was attached, and a charge-only cable will not show a serial port. The sketch will not erase while it is still waiting for serial.
7. Upload only the provisioner. Replace the port with the one the host actually enumerates:

   ```sh
   python3 -m platformio run -e xiaoble_provision -d firmware -t upload --upload-port PORT
   ```

8. Open the serial monitor at 115200. Opening it may reset the board. Wait until the full warning block is on screen. If it is not, stop. Do not type the phrase from memory into a blank monitor.
9. Type `ERASE SLAB FS` and press Enter. If the monitor prints `REFUSED`, flash was not erased. Reset and return to step 8 if that was a typo. If it prints `PROVISION FAILED`, stop using the board for scoring and return to step 7.
10. Continue only after the monitor prints `PROVISION OK` and lists all three paths as `absent`.
11. Upload the scoring firmware, not the provisioner:

    ```sh
    python3 -m platformio run -e xiaoble -d firmware -t upload --upload-port PORT
    ```

12. First scoring boot. Required evidence, not yet collected: a photo of the panel showing `QUICK START` and `0 SAVED / MODE TO VIEW`. `STORAGE ERROR` or `NO DATA ERASED` is a stop. Do not look for those words on serial; the scoring image does not print them.
13. Reset or power-cycle once. Second boot must still show `QUICK START` and `0 SAVED`, not `STORAGE ERROR`. Photograph that boot too. If the second boot shows `STORAGE ERROR`, stop. The journal write on first boot did not come back as a valid store, and scoring firmware will not repair it by formatting.
14. Stop. Do not start a round, do not erase again, and do not describe the board as storage-qualified. Flash wear, power-cut, and 30-day retention are outside this procedure.

## Versions pinned for the bench images

Both `xiaoble` and `xiaoble_provision` extend the same platform pin in `firmware/platformio.ini`:

| Piece | Pin |
| --- | --- |
| Board | `xiaoble_adafruit` (Seeed XIAO nRF52840, Adafruit nRF52 Arduino core) |
| Platform | `https://github.com/maxgerhardt/platform-nordicnrf52.git#cac6fcf943a41accd2aeb4f3659ae297a73f422e` (10.1.0, recorded earlier as `10.1.0+sha.cac6fcf`) |
| Framework package | `framework-arduinoadafruitnrf52-seeed` at `3ae21c3c6c544fed6e949f00986324dd95fb2b89` (Seeed core 1.1.1, recorded earlier as `1.10101.0+sha.3ae21c3`) |
| Toolchain package | `toolchain-gccarmnoneeabi@1.70201.0` (GCC ARM 7.2.1) |
| Display library, scoring env only | `zinggjm/GxEPD2@1.6.9` |
| Provisioner env | no GxEPD2 dependency; `src_dir = tools/provision`; `-DSLAB_FACTORY_PROVISION=1` |

The scoring env name remains `xiaoble`. Its source filter and `main.cpp` mount call are unchanged aside from the platform and GxEPD2 pins above. Those pins replace the floating platform git URL and `GxEPD2@^1.5.8` so a later resolve cannot move the core that owns `InternalFileSystem::begin()`.

Package versions and image hashes from the build that produced this commit are in the build record below. If that record still says pending, the images have not been identified.

## Build record

Filled from the PlatformIO build in this workspace. These hashes identify the linked files. They are not a device measurement.

| Item | Value |
| --- | --- |
| PlatformIO Core | pending |
| Resolved platform | pending |
| Resolved framework | pending |
| Resolved toolchain | pending |
| Resolved GxEPD2 | pending |
| `firmware/tools/provision/check_no_autoformat.py` | pending |
| `make -C firmware/host test` | pending |
| SHA-256 `xiaoble_provision` firmware.hex | pending |
| SHA-256 `xiaoble_provision` firmware.bin | pending |
| SHA-256 `xiaoble` firmware.hex | pending |
| SHA-256 `xiaoble` firmware.bin | pending |

Linker size lines, if the build prints them, belong in the commit that fills this table. RAM and flash figures from the linker are not runtime stack or heap measurements.

## Unverified on silicon

No claim of a successful provision, a successful first boot, or a successful second boot is made.

Still required before anyone treats a board as provisioned:

- A data-capable USB-C cable enumerated a serial port on Brian’s XIAO. Not tried.
- Serial transcript of the warning block, the typed phrase, and `PROVISION OK`, including the three `absent` lines. Not captured.
- Photo of the scoring panel on first boot showing `QUICK START`, not `STORAGE ERROR` / `NO DATA ERASED`. Not captured.
- Photo after a second reboot showing `QUICK START` and `0 SAVED` again. Not captured.
- LED on/off while waiting, erasing, refused, failed, or done. The sketch uses `LED_STATE_ON` from the XIAO variant header. That polarity was not observed.
- Real page-erase, `lfs_format`, and mount of internal flash. Compile and host journal tests do not do this.
- Power loss during erase or during the first journal save. Not tested. If erase is interrupted, expect `PROVISION FAILED` or, under scoring firmware, `STORAGE ERROR`. The recovery is to run this provisioner again, not to boot scoring firmware repeatedly.
