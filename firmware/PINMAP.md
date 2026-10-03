# Firmware pin map (current bench → XIAO nRF52840)

Seeed XIAO nRF52840 (plain, not Sense) + Waveshare 2.9" e-Paper module.

## E-ink (4-wire SPI)

Stock library only: **GxEPD2_290_T94_V2**. No custom SSD1680 driver.

| Module | XIAO | Notes |
| --- | --- | --- |
| VCC | 3V3 | 3.3 V |
| GND | GND | |
| DIN | D10 (MOSI) | SPI MOSI |
| CLK | D8 (SCK) | SPI SCK |
| CS | D7 | low active |
| DC | D5 | high=data, low=cmd |
| RST | D4 | low active |
| BUSY | D3 | **high = busy** on the Waveshare module |

MISO is not wired to the panel. The SPI peripheral's MISO pin is D1 so it does not take NEXT (D9).

Partial refresh for stroke/putt value edits; full refresh on state change / NEXT / confirm. BLE advertises only on the sync screen.

## Buttons (to GND, INPUT_PULLUP)

| Button | GPIO | Behavior |
| --- | --- | --- |
| MODE | D0 | short: back one step, or reopen only the hole just finished; hold ~2s: deep sleep. MODE wakes. |
| + | D2 | strokes up from par; on the drive screen, Left (or back to Fairway from Right) |
| − | D6 | strokes down from par; on the drive screen, Right (or back to Fairway from Left) |
| NEXT | D9 | advance. On the sync screen, hold NEXT for a new round |

PUTT is not part of this UX and is not read. Strokes on the demo card open at par. Drive on par 4/5 opens on Fairway.

Software debounce **25 ms**. MODE hold **2 s**. NEXT long-press **600 ms**.

## Persistence

LittleFS journal `/slab_store.0` and `/slab_store.1`. An empty queue is filled with one completed demo round on boot. See `firmware/BLE_SYNC.md`.

## Power

- USB-C for charge and UF2 flash
- Hold MODE about 2 seconds to enter System OFF. Press MODE to wake (the chip resets)
- The panel is hibernated before sleep

## Out of scope

Cellular, rangefinder, shot tracking, AI caddie, touchscreen-first UI, on-device handicap, live GHIN.
