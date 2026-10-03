# Slab BLE sync (XIAO nRF52840 bench)

Flashable image for a plain Seeed XIAO nRF52840 (not the Sense board). The display path is still stock `GxEPD2_290_T94_V2`. Advertising runs only on the post-round sync screen.

## Build

```sh
python3 -m platformio run -e xiaoble -d firmware
```

| Item | Value |
| --- | --- |
| PlatformIO env | `xiaoble` |
| Board | `xiaoble_adafruit` (Adafruit nRF52 core + LittleFS, not the mbed `xiaoble` target) |
| Framework | `framework-arduinoadafruitnrf52-seeed` @ `3ae21c3` |
| Platform | `maxgerhardt/platform-nordicnrf52` @ `cac6fcf` |
| Local name | `Slab` |
| UF2 | `firmware/dist/slab-xiaoble.uf2` |
| UF2 family | `0xADA52840` (Adafruit nRF52840). Application start `0x27000` (S140 7.3.0) |
| UF2 sha256 | `9af45c63908e6da5618422c6be82d396a8c5539dd359fcdc66853b7ac2ecb1bb` |

## Flash

1. Use a **plain XIAO nRF52840**, not the Sense board.
2. Double-tap RST. A drive named `XIAO-BOOT` appears.
3. Copy `firmware/dist/slab-xiaoble.uf2` onto that drive. The board reboots when the copy finishes.
4. Optional: serial monitor at **115200**. The banner prints `Slab BLE name=Slab` and the oldest round id.

The factory erase image is still `xiaoble_provision`. Do not flash that when you want this scorer.

## What you should see

A fresh or empty journal gets one completed 18-hole round (the same scores as `protocol/fixtures/eighteen.json`: 74 strokes, fairway where the hole is not a par 3). The panel opens on the sync screen with `BLE SLAB / HOLD NEXT NEW`. The course label stored on the round is `DEMO` / `PAR72` (the pars already in firmware, sum 72). That label is not inside the v2 JSON body.

## nRF Connect (iPhone)

1. Open nRF Connect. Scan. Connect to **Slab**.
2. Open service `534c4201-0000-4e52-8000-00805f9b34fb`.
3. Read **Slab round meta** (`534c4202-0000-4e52-8000-00805f9b34fb`). It is UTF-8 JSON:

```json
{"schema_version":2,"round_id":"slab-0123456789abcdef:00000001","device_id":"slab-0123456789abcdef","round_sequence":1,"checksum":"crc32:........","byte_length":1387,"page_length":512,"long_read":0}
```

`round_id` and `device_id` use this board's id (`slab-` plus 16 hex digits). The first preloaded round is **1387** bytes. `checksum` changes per board. `long_read` is 0: S140 will not store an attribute larger than 512 bytes, so there is no single JSON characteristic.

4. Read these three characteristics and concatenate the UTF-8 text with nothing added between them. nRF Connect assembles each long value when you tap Read.

| Order | Name | UUID | Bytes |
| --- | --- | --- | --- |
| 1 | Slab JSON page 0 | `534c4210-0000-4e52-8000-00805f9b34fb` | 512 |
| 2 | Slab JSON page 1 | `534c4211-0000-4e52-8000-00805f9b34fb` | 512 |
| 3 | Slab JSON page 2 | `534c4212-0000-4e52-8000-00805f9b34fb` | 363 |

Pages 3–7 (`534c4213` … `534c4217`) are empty. The joined text is one protocol-v2 object (`"schema_version":2`) whose `checksum` matches meta. Do not insert spaces or newlines.

5. Optional chunk window (`534c4204-…`): a Read with no write returns the first 180 bytes. Write a 4-byte little-endian offset (`00-00-00-00`, or ASCII `180`), wait a moment, then Read. Offset 0 again restarts the window.

## ACK (retires the oldest round only)

Write the **meta JSON text** to **Slab round ACK** (`534c4205-0000-4e52-8000-00805f9b34fb`). The extra `byte_length` / `page_length` / `long_read` keys are ignored. The five protocol fields are checked against the oldest queued round.

Wait about a second, then read **Slab ACK result** (`534c4206-…`):

| Text | Meaning |
| --- | --- |
| `ok` | Journal committed and that round is gone |
| `conflict` | Id, sequence, or checksum did not match the oldest round |
| `not_found` | Queue was empty |
| `invalid` | JSON was not a v2 ACK |
| `persist_error` | Store was left unchanged |

A repeated ACK for a round that is already gone does not delete the next one. There is no pairing. Treat this image as a bench tool.

Still open: a transfer-begin session, gap detection beyond this window, and an ACK that waits on the phone's own durable commit.

## Put the demo round back

- Serial **115200**, send `seed` and a newline. That appends another completed demo round when nothing is in progress and the queue is not full (10 rounds). The sync screen comes back.
- Reboot with an empty queue does the same thing once.
- Hold NEXT on the sync screen to open Quick Start. That does **not** delete the queued demo round. Short MODE from Quick Start shows it again.
- Hold MODE about 2 seconds on any screen for System OFF. Press MODE to wake. The wake press is ignored until you release it, so the board does not immediately sleep again.

## Playing a real round

Quick Start uses the same par-72 card. Strokes open at that hole's par. On the drive screen, Fairway is selected; + is Left, − is Right, and the opposite button returns to Fairway. Par 3 has no drive. MODE on the first screen of a hole, before you change it, reopens only the hole you just finished.
