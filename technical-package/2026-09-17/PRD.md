# Slab PRD (engineering input, 2026-09-17)

**Status:** internally consistent review draft. Not frozen. Not production-ready.  
**Supersedes for this review:** stale `main` claims in `docs/SPEC.md`, `docs/FIRMWARE_PLAN.md`, `docs/PHONE_APP.md`, `docs/RISKS.md`, `README.md` (steel plate in box, MODE-to-GIR, IP67-ish product, `<$40/@100` / `<$15/@5k`).  
**Revision date:** 2026-09-17

## 1. Product boundary

Slab is a **cart-mounted e-paper score puck**. The golfer plays the round on the puck. The phone app imports completed rounds later. The puck is the in-round source of truth.

| In | Out |
| --- | --- |
| Offline 9/18-hole scoring on device | Rangefinder, GPS, shot tracking, AI caddie |
| Strokes, putts, drive L / fairway / R on par 4/5 | Dedicated GIR pad or MODE-to-GIR prompt |
| Derived GIR: `(strokes − putts) ≤ (par − 2)` | On-device handicap |
| Local save through sleep / power loss (required; **not evidenced**) | Live phone scoreboard / mid-round BLE |
| App-open BLE import of queued rounds | Background-sync promise |
| Direct magnet + removable clamp | Steel plate + VHB **in the box** |
| Signed BLE DFU in **production** (required; **not implemented**) | Cellular, touchscreen-first UI |
| Export/post later via a future partner | GHIN affiliation, live GHIN, invented public GHIN API |

Website (`playslabgolf.com`) and the PR #3 visual app are **not** hardware proof. They are cited only where they define puck/app interface or product rules.

## 2. Users and jobs

- **On the first tee:** start a 9 or 18 hole round in seconds. Course preload is helpful metadata, not an entry ticket. **Intended inactive / start / course-card boot is NOT implemented** (firmware still boots `LOCAL EXAMPLE`).
- **During play:** glance hole / par / score; edit strokes then putts then confirm; phone stays away. BLE off.
- **Walking off 18:** save on Slab first. Sync is optional and later.
- **Later (parking lot / days later):** open the app, import queued rounds oldest-first, review, optional humble coaching inside captured evidence.

## 3. In-round flow (locked interaction)

Sequential lock (PR #1 `slab_apply_event`): **strokes → putts → done**. No GIR prompt. PUTT pad is wired and **ignored** (cannot skip to putts).

```
DEFAULT_HOLE
  NEXT → STROKE_EDIT     (+/− change strokes; min 1)
  NEXT → PUTTS_INPUT     (+/− change putts)
  NEXT → END_HOLE_CONFIRM
           par 3: drive N/A
           par 4/5: cycle drive L / fairway / R
           NEXT locks hole and advances
           after last hole → ROUND_COMPLETE_SYNC (BLE advertise legal)
           MODE from sync → DERIVED_STATS (GIR / putts computed)
```

Defaults when a hole is created: strokes = par, putts = 2, fairway = fairway on par 4/5 else N/A.

**Not implemented (required product, missing firmware):** Quick Start vs course-card load, explicit 9/18 start, End-now-as-9 at the turn, “ROUND SAVED ON SLAB” before sync copy, multi-round queue, ACK-after-durable-write, DFU.

## 4. Puck / app contract (v0.1)

Authoritative notes: `PUCK-APP-CONTRACT-NOTES.md`. Sources: PR #3 `puck-app-round-contract-v0.1.md` + `round-contract.ts` at `57609af3f7e23819a89e01e5a803a8f36a80f5b8`. Firmware payload stub: PR #1 `slab_ble_build_payload` at `e7f7ad79c885627616819ad08114a704024569f7`. Main `docs/SYNC_PAYLOAD.md` is **older and inconsistent**.

**Agreed product rules**

- Advertise / transfer only after the golfer completes the round (`ROUND_COMPLETE_SYNC`). BLE off during play.
- App-open import. ACK only after a validated durable write. Never delete on transmit alone.
- Idempotent by `round_id`. Duplicates update sync metadata only.
- GIR is derived; not a captured field the golfer enters.
- Preserve unknown-schema payloads; require app update; do not discard.
- Edit provenance: captured-on-Slab vs edited-in-app.

**Open contract tension (do not silently pick)**

| Topic | `docs/SYNC_PAYLOAD.md` + PR #1 builder | PR #3 v0.1 contract |
| --- | --- | --- |
| Schema id | `"schema": "stat-puck.round.v1"` | `"schema_version": 1` |
| GIR | boolean **in** payload (device-derived) | **omitted**; app derives |
| Drive | `fairway`: `H`/`L`/`R`/`NA` | `drive_result`: `left`/`fairway`/`right`/`null` |
| Checksum / sequence | absent | required `checksum`; sequence mentioned in prose |
| Totals / timestamps | `synced_at` / `date` / `score_type` | `totals`, optional `started_at`/`completed_at` |
| Course | object on device | may be attached later in app |

**DFU:** neither contract defines a DFU GATT service or state machine. **NOT IMPLEMENTED.** Must be a separate versioned service. See `BLE-DFU.md`.

## 5. Mechanical / electrical targets

Owner-approved values are requirements. Proposed values are labeled. Details and blank decision column: `TARGET-VALUES-FOR-OWNER-REVIEW.md`.

| Area | Value | Status |
| --- | --- | --- |
| Battery outcomes | ≥48 h continuous active-round **and** ≥21 calendar days typical use (9×5 h + 458.25 h sleep + 9×5 min sync) | **Owner-approved** |
| Cell size | Optimize downward from measured currents; usable ≤80% nameplate first pass; 500 mAh is P1 assumption **not a floor** | **Owner-approved** |
| Ingress | IP55 IEC 60529; not IP67 / not submersion | **Owner-approved** |
| Drop | 1.8 m / 6 ft smooth concrete; 6 faces + 4 corners = 10 drops; Instinct pass criteria | **Owner-approved** |
| BLE | Reliable sync at 5 m around cart; 10 m LOS verification; off during play | **Owner-approved** |
| Display refresh | Vendor-typical full ~3 s / partial ~0.3–0.6 s (Waveshare note + PR #1 constants). Not measured. | **Proposed** |
| Op / storage temp | 0–40 °C operate / −25–60 °C store (e-paper-class proposal, not tested) | **Proposed** |
| Mount vibration | Remain retained on magnet **and** clamp through representative cart use; no invented g-level | **Proposed** |
| Envelope | Face 110 × 58 mm; thickness ≤18 mm; mass ≤150 g | **Proposed** |
| Buttons | 5 pads; wet-glove usable; durability per selected switch after DFM | **Proposed** |
| Unsynced retention | ≥10 completed rounds and ≥30 days | **Proposed** |
| Compliance design-in | US + EU (FCC + CE/RED). Not certified. | **Proposed** |
| Production DFU | Signed BLE DFU; brick-safe; see `BLE-DFU.md` | **Owner-approved requirement; not implemented** |

**P1 electrical starting point (not production freeze)**

- MCU+BLE: Seeed XIAO nRF52840 (DigiKey 102010448). Production candidate: Raytac MDBT50Q-1MV2 (nRF52840).
- Display: Waveshare 2.9" B/W 296×128, SSD1680, 4-wire SPI.
- Buttons: 5× Omron B3W-1000 (switch IP67 excluding terminals — **not** a product IP rating).
- Battery concept: Adafruit 1578 3.7 V 500 mAh protected LiPo. Charge via XIAO USB-C / BQ25101 on P1.
- Magnet: K&J DC6TP-N52 rubber-coated N52, Ø19.05 × 9.52 mm. Vendor Case-1 pull **13.12 lb** is a catalog figure, not a cart-retention test.

No custom schematic exists.

## 6. Firmware / app implementation boundary

| Layer | Today | Required |
| --- | --- | --- |
| Host visual parity | PR #1 host renderer vs Instinct 296×128 goldens | Keep as UI lock, not hardware proof |
| XIAO build | PR #1 claims `pio run -e xiaoble -d firmware` SUCCESS | Bring-up on real XIAO + panel still required |
| Real panel | **Not tested in-repo** | EVT display evidence |
| BLE GATT / course-card / E2E transfer | Stub advertise gate + JSON builder only | E2E import + ACK |
| Multi-round queue / power-loss | LittleFS hooks; **no evidence** | Required product behavior |
| BLE DFU | **NOT IMPLEMENTED** | Production requirement |

## 7. Non-goals (v1 hardware + import)

- Live GHIN, official handicap, public GHIN API
- Dedicated GIR input
- Steel plate kit in the box
- Certified weatherproofing language (design **to** IP55; do not claim certified)
- Cost promise `<$40 @ 100` / `<$15 @ 5k`
- Ship dates, preorders, production pricing
- Background BLE sync as the v1 promise
- Shot distance, clubs, penalties, full strokes-gained, social feeds
- Over-specific coaching the puck did not collect

## 8. Open-requirements table

| ID | Requirement | Owner status | Evidence today | Open decision |
| --- | --- | --- | --- | --- |
| BAT-01 | 48 h active-round + 21-day typical | Approved | None measured | Cell / charger / thickness after EVT currents |
| BAT-02 | First-pass nameplate = formula / 0.80 | Approved | Formula only | I_active / I_sleep / I_sync |
| BAT-03 | Compare 150/200/250/300/500 mAh | Approved (comparison) | Catalog examples only | Smallest passing protected cell |
| IP-01 | IP55 IEC 60529 | Approved | None | USB-C + button + split-line method |
| DRP-01 | 1.8 m concrete × 10 drops | Approved | None | Glass/film protection vs ≤18 mm / ≤150 g |
| BLE-01 | 5 m around cart + 10 m LOS | Approved | None | Antenna vs magnet / cart steel |
| BLE-02 | BLE off during play | Approved | Code gate only | RF implementation |
| DFU-01 | Signed BLE DFU + recovery | Approved | **Not implemented** | Nordic Secure DFU / MCUboot / other |
| CON-01 | Unify v0.1 payload | Open | Two schemas | Which schema ships |
| Q-01 | ≥10 rounds / 30-day queue | Proposed | None | NVM budget + LittleFS vs external |
| ENV-01 | Envelope 110×58 / ≤18 mm / ≤150 g | Proposed | CAD 110×58×15.5 concept | Conflict with BAT/IP/DRP |
| ENV-02 | Op/storage temperature | Proposed | Vendor panel notes | Golf outdoor envelope |
| MNT-01 | Magnet + clamp retention / vibe | Proposed | Concept CAD only | Mixed-material carts |
| DSP-01 | Full/partial refresh budget | Proposed | Vendor typical + `#define`s | Production panel |
| BTN-01 | Wet-glove + life | Proposed | B3W datasheet only | Cap design |
| CMP-01 | US+EU design-in | Proposed | None | Module vs discrete radio |
| CAD-01 | Neutral assembly STEP | Blocked | Files missing | Export or rebuild CAD |
| PCB-01 | Custom PCB / schematic | TBD | None | P1 wiring vs production board |
| CHG-01 | Charge time | TBD | None | Charger IC + cell |

## 9. Risks that must stay visible

- Battery target may force thickness/mass above the proposed envelope — **do not silently weaken BAT-01**.
- 10× 1.8 m concrete drops may conflict with e-paper glass and ≤18 mm / ≤150 g — flag; do not drop the height.
- Magnet next to BLE antenna and cart steel is unvalidated.
- `cad/README.md` over-claims `device_assembly.step/.stl`.
- Stale main docs will confuse Peakingtech if sent instead of this folder.
