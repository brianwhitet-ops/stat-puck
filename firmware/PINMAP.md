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

Partial refresh for stroke/putt edits; full refresh on NEXT. Never invent mid-round network calls.

## Buttons (Omron B3W-1000 → GND, INPUT_PULLUP)

| Button | GPIO | Behavior |
| --- | --- | --- |
| + | D2 | strokes++ |
| − | D4 | strokes−− (min 1) |
| PUTT | D5 | putts++; long-press putts−− |
| MODE | D0 | cycle FWY then GIR |
| NEXT | D9 | lock hole / advance (hold confirm on 9 & 18) |

Software debounce ≥20 ms (datasheet bounce ≤5 ms).

## Power

- JST LiPo on XIAO charge pad / battery pads per Seeed wiki
- USB-C for charge + UF2 flash
- Sleep between IRQs; power down panel between refreshes

## Out of scope

Cellular, rangefinder, shot tracking, AI caddie, touchscreen-first UI, on-device handicap, live GHIN.
