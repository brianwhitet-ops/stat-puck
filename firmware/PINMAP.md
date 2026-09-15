# Firmware pin map (P1 breadboard → XIAO nRF52840)

Based on Seeed XIAO nRF52840 + Waveshare 2.9" e-Paper module datasheet pinout.

## E-ink (4-wire SPI)

| Module | XIAO | Notes |
| --- | --- | --- |
| VCC | 3V3 | 3.3 V |
| GND | GND | |
| DIN | D10 (MOSI) | SPI MOSI |
| SCLK | D8 (SCK) | SPI SCK |
| CS | D7 | low active |
| DC | D6 | high=data, low=cmd |
| RST | D3 | low active |
| BUSY | D1 | **high = busy** on Waveshare module |

Partial refresh for stroke/putt value edits; full refresh on state change / NEXT / confirm. BLE advertise only in `ROUND_COMPLETE_SYNC`.

Controller lock: **SSD1680** via **GxEPD2_290_T94_V2** (Waveshare 2.9" B/W V2/V3). See `firmware/README.md`.

## Buttons (Omron B3W-1000 → GND, INPUT_PULLUP)

Sequential lock: strokes → putts → done. No GIR pad.

| Button | GPIO | Behavior |
| --- | --- | --- |
| + | D2 | increment active field (enters `STROKE_EDIT` from default) |
| − | D4 | decrement (min strokes 1) |
| PUTT | D5 | wired, **ignored** (cannot skip to putts) |
| MODE | D0 | back one step; cycle drive L/FWY/R on confirm (par 4/5); toggle stats after 18 |
| NEXT | D9 | advance: default→stroke→putts→confirm→next hole / sync |

Software debounce **25 ms** (datasheet bounce ≤5 ms). Optional dial (unpopulated) shares the +/− path.

## Persistence

On-device LittleFS `/slab_round.bin` (18-hole card + UI). Host bench uses `firmware/testdata/nvram.bin`.

## Power

- JST LiPo on XIAO charge pad / battery pads per Seeed wiki
- USB-C for charge + UF2 flash
- Sleep between IRQs; power down panel between refreshes

## Out of scope

Cellular, rangefinder, shot tracking, AI caddie, touchscreen-first UI, on-device handicap, live GHIN.
