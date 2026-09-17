# Slab — Peakingtech technical-review package

**Audience:** Brian + Instinct (internal). **Not for Peakingtech yet.**  
**Revision date:** 2026-09-17  
**Branch:** `cursor/peakingtech-tech-package-0917`  
**Package path:** `technical-package/2026-09-17/`  
**Manifest:** `technical-package/2026-09-17/MANIFEST.md`

## Purpose

This folder is engineering **input** for a later Peakingtech technical review: product boundary, architecture, mechanical brief, firmware truth, BOM, evidence, and validation gaps.

It is **not** a frozen design transfer, production-ready package, quote, or authorization to choose parts or spend. Existing CAD, firmware, and app work are concept / bench inputs. Host renders and a successful PlatformIO build are **not** hardware proof.

Engagement shape (already stated to Peakingtech, not restated here as a new decision): Peakingtech would complete and validate architecture before EVT; current Slab work is input, not a locked handoff. This package exists so Brian and Instinct can review that input first.

## Honesty locks

Do not treat anything in this package as a claim of:

- measured currents, battery life, BLE range, drop, ingress, temperature, or charge time
- production unit cost, NRE, MOQ, ship date, preorder, or certified weatherproofing
- GHIN affiliation, live GHIN, or a public GHIN API
- a dedicated GIR button / MODE-to-GIR prompt (GIR is **derived** only)
- a steel plate in the box (mount is **direct magnet + removable clamp**)
- an unvalidated `<$40 @ 100` / `<$15 @ 5k` cost promise
- implemented OTA or BLE DFU (PR #1 and PR #3: **NOT IMPLEMENTED**)

Stale `main` docs (`docs/SPEC.md`, `docs/RISKS.md`, `docs/FIRMWARE_PLAN.md`, `README.md`, `cad/README.md`) still mention steel plate, IP67-ish sealing, MODE-to-GIR, and cost targets. **This package supersedes those claims** for review. Do not send `main` as-is to Peakingtech.

## What we are asking Peakingtech to advise on

Advice only. Not authority to select suppliers, freeze the architecture, or spend.

### a) Battery / cell / charging for a cart-mounted e-paper device

Owner-approved outcomes (not a 500 mAh floor):

- ≥48 h continuous **active-round** operation per charge
- ≥21 calendar days typical use: 3 rounds/week × 3 weeks = 9 rounds / 45 active hours
- Duty cycle: 5 h/round active; e-paper hold ≈ 0; 100 partial + 20 full refreshes/round; BLE **off** in play; **458.25 h** sleep in the 21-day window; 9 × 5 min BLE sync
- First-pass usable capacity ≤ 80% of nameplate
- Size downward from measured currents. 500 mAh (Adafruit 1578) is the P1 **starting assumption**, not a minimum.

Formula (first pass):

`max(48h × I_active, 45h × I_active + 458.25h × I_sleep + 0.75h × I_sync) / 0.80`

Please advise: cell chemistry/format, protection vs pack vs board, charger IC vs XIAO BQ25101, USB-C charge path with IP55, cold/aging derating, and the smallest commercially suitable protected cell that can pass both gates after EVT measurement. Compare at least 150 / 200 / 250 / 300 / 500 mAh candidates.

### b) Direct magnet vs removable clamp under cart vibration / mixed materials

Mount is **direct magnet + removable clamp**. There is **no steel plate in the box**.

Please advise retention on mixed cart materials (ferrous and non-ferrous / plastic rental bodies), vibration, magnet–antenna interaction, clamp bore/tolerance for typical Club Car / EZ-GO bars, and whether either path fails the 1.8 m drop or 5 m BLE gates.

### c) Production panel sourcing vs Waveshare 2.9" P1

P1 module: Waveshare 2.9" B/W 296×128, SSD1680, GxEPD2_290_T94_V2 / GDEM029T94 family. Module outline in CAD is 89.5 × 38 mm (vendor note).

Please advise a production panel/controller path (bare panel vs module), availability, optical/mechanical stack, refresh energy, and whether to stay on SSD1680.

### d) Sealing around buttons, split line, USB-C for IP55

Owner target: **IP55 per IEC 60529** (rain/splash/jets). **Not IP67 / not submersion.**

Please advise gasket/split-line, button-cap vs IP67-switch-only, USB-C cap vs sealed connector vs service-only charge, and whether IP55 is compatible with serviceability.

### e) FCC/CE design-in / pre-scan / certification path

Proposed launch design-in: US + EU (FCC + CE/RED). **Not certified.** No test report exists.

Please advise module vs discrete radio, antenna keep-out around the N52 magnet and cart steel, pre-scan order, and what must be frozen before a quote.

### f) BLE DFU architecture for the selected MCU

Production requirement: signed app-to-puck firmware update over BLE. **Not implemented** on PR #1 (commit `e7f7ad79c885627616819ad08114a704024569f7`) or PR #3 (`57609af3f7e23819a89e01e5a803a8f36a80f5b8`). MCU BLE capability ≠ OTA.

Please advise: Nordic Secure DFU / MCUboot vs another proven signed path; dual-bank vs immutable recovery; flash/storage overhead on nRF52840 (XIAO P1 / Raytac MDBT50Q-1MV2 production candidate); signing/provisioning ownership; app integration (separate DFU GATT, not the round payload). See `BLE-DFU.md`.

## Package contents

| File | Role |
| --- | --- |
| `MANIFEST.md` | Every package file + cited source → repo path + commit SHA |
| `PRD.md` | Product boundary, flows, targets, puck/app contract, non-goals, open-requirements table |
| `TARGET-VALUES-FOR-OWNER-REVIEW.md` | Owner-approved vs proposed values; blank owner-decision column |
| `ARCHITECTURE.md` | Block diagram + interface table; P1 vs production |
| `MECHANICAL.md` | Dimensioned brief; STEP/STL honesty and CAD blocker |
| `FIRMWARE-STATUS.md` | Truth matrix vs PR #1 `e7f7ad79…` |
| `BOM.csv` | Engineering BOM (P1 / production / TBD) |
| `EVIDENCE-INDEX.md` | Datasheet links + which requirement each supports |
| `VALIDATION-MATRIX.md` | Gates; absent evidence = TBD |
| `BLE-DFU.md` | Production DFU requirement; current PRs not implemented |
| `PUCK-APP-CONTRACT-NOTES.md` | v0.1 import contract + **missing DFU state machine** |

## Known blockers (this revision)

1. **`cad/device_assembly.step` and `cad/device_assembly.stl` do not exist** on `main` (`fe7a88f100afa7f3d60271ff91e7fc2c33b08f49`). `cad/README.md` claims them. Present: enclosure top/bottom STEP+STL, magnet STEP+STL, optional clamp STEP+STL. Tiny STLs (`pcb.stl`, `eink.stl`, `battery.stl`, `xiao.stl`, `gasket.stl`) are **placeholder boxes**, not production geometry. `cad/steel_plate.stl` is a **superseded concept** (not in the box).
2. **No schematic / custom PCB / Gerbers.** P1 is XIAO + module wiring (`firmware/PINMAP.md`).
3. **No real-panel, measured-power, BLE range, drop, ingress, temperature, power-loss, or DFU evidence.**
4. **Instinct draft PRD / architecture / firmware-matrix / BOM files** mentioned on PR #2 were not in this checkout or Google Drive search; this package is rebuilt from owner comments + repo/PR sources. If Brian has those drafts, they should be diffed against this folder.
5. **Puck/app payload schemas disagree** (`docs/SYNC_PAYLOAD.md` + PR #1 builder vs PR #3 `schema_version: 1` contract). Documented in `PRD.md` / `PUCK-APP-CONTRACT-NOTES.md`. Not silently merged.

## Distribution

Internal review only. Do not email or upload this folder to Peakingtech until Brian says so (NDA status is a separate thread and is not complete in this package).
