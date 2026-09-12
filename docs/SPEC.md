# Stat Puck — One-Page Spec

**Product:** Magnetic golf-cart score puck (Teenage Engineering–style slab)  
**Prototype codename:** `stat-puck`  
**Sync model:** Offline during round → BLE pull to phone after 18 (or 9) → phone posts later  

---

## Mechanical

| Item | Spec |
| --- | --- |
| Envelope | 110 × 58 × 15.5 mm, rounded slab, matte charcoal |
| Mass target | < 150 g (prototype estimate ~95–120 g) |
| Mount | N52 rubber-coated disc (K&J **DC6TP-N52**, Ø19.05 × 9.52 mm, **13.12 lb** Case-1 pull) in bottom pocket |
| Rental carts | Include 70 × 40 × 1.2 mm steel plate + 3M VHB tape (no ferrous metal on plastic bodies) |
| Optional | Secondary clamp for Ø22–28 mm Club Car / EZGO bar |
| Sealing | Silicone gasket in split line + IP67 Omron **B3W** switches; target **IP67-ish** (not lab-certified on rev A) |
| Aesthetic | Hidden M2 fasteners, no toy colors, no pelican ribs |

## Electrical

| Block | Choice | Notes from datasheets |
| --- | --- | --- |
| MCU + BLE | Seeed **XIAO nRF52840** (DigiKey 102010448) for P1; production → Raytac **MDBT50Q-1MV2** | USB-C, BLE 5.x, BQ25101 charger on XIAO; Raytac: 10.5×15.5×2.05 mm, VDD **1.7–5.5 V**, Nordic nRF52840 |
| Display | Waveshare **2.9" e-Paper** 296×128 | Panel outline **36.7×79.0×1.05 mm**; module **89.5×38 mm**; **4-wire SPI**; full refresh ~3 s, partial ~0.6 s; standby ≈0 |
| Buttons | 5× Omron **B3W-1000** | **IP67** (except terminals), bounce **≤5 ms**, OF **1.57 N** |
| Battery | Adafruit **1578** 3.7 V **500 mAh** LiPo w/ protection | 36×29×4.75 mm; charge via XIAO |
| Magnet | K&J **DC6TP-N52** | Rubber coat, water-resistant, non-scratch |

## Firmware states

```
BOOT → IDLE_PAIR → LOAD_CARD → IN_ROUND → TURN_SYNC? → END_ROUND_SYNC → SLEEP
```

Defaults: strokes=par, putts=2, GIR suggest `(strokes−putts)≤(par−2)`, FWY N/A on par 3. No on-device handicap. No live GHIN.

## Phone app (specify only)

Pick course + tees → push pars; pull card; one-screen edits; export payload; GHIN auth later (partner). Never invent a public GHIN API.

## BOM cost targets

| Qty | Target |
| --- | --- |
| 100 | **< $40** |
| 5 000 | **< $15** |

P1 shopping cart ~$70–90 (dev modules).

## Risks

Rental-cart plastic (need steel plate); e-ink lag (partial refresh); wet bounce (IP67+debounce); accidental NEXT (hold confirm); double-post (`round_id`); GHIN partner-gated.
