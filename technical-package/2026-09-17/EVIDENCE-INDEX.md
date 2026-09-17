# Evidence index

**Revision:** 2026-09-17  
**Rule:** `datasheets/` in this repo contains **only** `README.md` (points at `docs/DATASHEET_NOTES.md`). **No manufacturer PDFs are checked in.** Do not treat concept JPEGs, assembly GIF, host goldens, or `pio run` as hardware evidence.

Redistribution: vendor PDFs stay at the manufacturer. This index uses canonical links. Revision/date is what the public page stated when noted; if a page has no visible rev, mark **rev unknown**.

| ID | Artifact | Link | Rev / date note | Supports | In-repo? |
| --- | --- | --- | --- | --- | --- |
| DS-XIAO | Seeed XIAO nRF52840 wiki / product | https://wiki.seeedstudio.com/XIAO_BLE/ ; DigiKey 102010448 | Wiki living doc; **rev unknown** | P1 MCU, USB-C, BLE, onboard charger mention | No PDF |
| DS-NRF | Nordic nRF52840 product | https://www.nordicsemi.com/Products/nRF52840 | Nordic product page; PS/OPL on Nordic site | Radio MCU, 1 MB flash class, BLE | No PDF |
| DS-RAY | Raytac MDBT50Q-1MV2 | https://www.raytac.com/product/ins.php?index_id=81 | Raytac product page; **rev unknown** | Production MCU **candidate** (10.5×15.5×2.05 mm, VDD 1.7–5.5 V per `docs/DATASHEET_NOTES.md`) | No PDF |
| DS-WS | Waveshare 2.9inch e-Paper Module | https://www.waveshare.com/2.9inch-e-paper-module.htm ; wiki https://www.waveshare.com/wiki/2.9inch_e-Paper_Module | Living wiki; catalog full ~3 s / partial ~0.6 s | P1 panel/module, SPI, refresh **typical** | No PDF |
| DS-SSD | SSD1680 controller (via GxEPD2 / Waveshare V2) | Solomon Systech SSD1680 materials on vendor site; PR #1 uses GxEPD2_290_T94_V2 | Controller lock in `slab_board.h` | P1 driver choice | No PDF |
| DS-GX | GxEPD2 library | https://github.com/ZinggJM/GxEPD2 (PR #1 `zinggjm/GxEPD2@^1.5.8`) | Library tag ^1.5.8 | Software driver, not a panel cert | Code on PR #1 only |
| DS-B3W | Omron B3W | https://www.omron.com/global/en/products-info/web/b3w/ ; DigiKey B3W-1000 | DigiKey listing; bounce ≤5 ms, OF 1.57 N, IP67 **except terminals** per `docs/DATASHEET_NOTES.md` | Switch, **not** product IP55 | No PDF |
| DS-1578 | Adafruit 1578 500 mAh | https://www.adafruit.com/product/1578 | Vendor page (size 29×36×4.75 mm, 10.5 g, 3.0 V cutoff) | P1 cell **assumption** | No PDF |
| DS-1317 | Adafruit 1317 150 mAh | https://www.adafruit.com/product/1317 | Vendor page (19.75×26.02×3.8 mm, 4.65 g) | 150 mAh **candidate example** | No PDF |
| DS-2750 | Adafruit 2750 350 mAh | https://www.adafruit.com/product/2750 | Vendor page (36×19.6×5.2 mm, 8.2 g) | Nearby catalog size (not required candidate) | No PDF |
| DS-KJ | K&J DC6TP-N52 | https://www.kjmagnetics.com/dc6tp-n52-neodymium-rubber-coated-disc-magnet | Catalog Case-1 **13.12 lb** | Magnet geometry / coat; **not** cart retention | No PDF |
| DS-IP | IEC 60529 | IEC Webstore (paid standard) | Standard | IP55 definition | Not redistributable here |
| DS-FCC | FCC / ISED equipment auth | FCC OET / TCB process pages | Process only | CMP-01 design-in — **no Slab grant** | None |
| DS-RED | EU RED / CE | Official Journal / ETSI EN 300 328 family | Process only | CMP-01 — **no Slab cert** | None |
| DS-NOTE | Internal digest | `docs/DATASHEET_NOTES.md` @ `fe7a88f` | 2026-09-12 era | Convenience only; prefer manufacturer | Yes |
| CAD-BOT | Enclosure bottom STEP | `cad/enclosure_bottom.step` @ `fe7a88f` | Added `91d8d84` | Concept envelope | Yes |
| CAD-TOP | Enclosure top STEP | `cad/enclosure_top.step` @ `fe7a88f` | Added `e628ff7` | Concept envelope | Yes |
| CAD-MAG | Magnet STEP | `cad/magnet.step` @ `fe7a88f` | | Concept cylinder | Yes |
| CAD-CL | Clamp STEP | `cad/optional_clamp.step` @ `fe7a88f` | | Concept clamp | Yes |
| CAD-MISS | Device assembly STEP/STL | `cad/device_assembly.step` `.stl` | **ABSENT** | Blocker | **No** |
| CAD-PH | Placeholder STLs | `cad/pcb.stl` `eink.stl` `battery.stl` `xiao.stl` `gasket.stl` | 684–1884 bytes | **Not** production geometry | Yes |
| CAD-STL-PLATE | Steel plate STL | `cad/steel_plate.stl` | | **Superseded** — do not use as requirement | Yes |
| FW-PR1 | Firmware tree | PR #1 `e7f7ad79c885627616819ad08114a704024569f7` | 2026-09-15 | SM, UI compose, persist hooks, BLE stub | Other branch |
| FW-GOLD | Instinct goldens | `design/instinct-firmware-goldens-pr1/` @ `e7f7ad79…` | SHA256SUMS on that commit | Host visual lock — **not** panel proof | Other branch |
| APP-C | v0.1 contract | PR #3 `57609af3f7e23819a89e01e5a803a8f36a80f5b8` | 2026-09-15 | Import rules / types | Other branch |
| IMG-CON | Concept renders | `concepts/web/*.jpg` @ `fe7a88f` | | Appearance only | Yes |
| IMG-GIF | Assembly preview | `docs/media/assembly-preview.gif` @ `fe7a88f` | | Appearance only | Yes |
| BOM-CART | P1 shopping cart | `bom/shopping_cart.csv` @ `fe7a88f` | ~2026-09-12 | List prices / links — **not** quotes | Yes |
| EV-PWR | Measured current traces | — | — | BAT-01 | **None** |
| EV-BLE | 5 m / 10 m RF log | — | — | BLE-01/02 | **None** |
| EV-DRP | Drop photos / functional log | — | — | DRP-01 | **None** |
| EV-IP | IEC 60529 test report | — | — | IP-01 | **None** |
| EV-DFU | Signed update logs | — | — | DFU-01 | **None** |
| EV-PANEL | On-glass photo / BUSY timing | — | — | DSP-01 | **None** |

## How to read this

- Rows DS-* support **part selection and typical vendor numbers**.
- Rows EV-* are the validation holes. Filling them is EVT work, not this package.
- If Peakingtech needs PDFs, download from the manufacturer under their terms. Do not assume we can zip copyrighted datasheets.
