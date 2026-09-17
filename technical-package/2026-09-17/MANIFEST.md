# Manifest — Slab technical package 2026-09-17

**Package revision date:** 2026-09-17  
**Branch:** `cursor/peakingtech-tech-package-0917`  
**Base `main`:** `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49`  
**First checkpoint commit (skeleton + cover):** `bc0c64d02ad60da5a0f7e064ba86e616a12139c3`  
**Fill commit:** recorded after push (see git log on this path)  
**Firmware (PR #1, not merged):** `e7f7ad79c885627616819ad08114a704024569f7`  
**App contract (PR #3, not merged):** `57609af3f7e23819a89e01e5a803a8f36a80f5b8`

Paths are repo-relative. Package-document SHAs are the last commit that changed that file on this branch.

## Package documents

| File | Repo path | Commit SHA | Notes |
| --- | --- | --- | --- |
| Cover | `technical-package/2026-09-17/COVER.md` | `bc0c64d02ad60da5a0f7e064ba86e616a12139c3` unless later amended | Internal; not for Peakingtech yet |
| This manifest | `technical-package/2026-09-17/MANIFEST.md` | last commit on this file | |
| PRD + open requirements | `technical-package/2026-09-17/PRD.md` | fill commit | |
| Target values | `technical-package/2026-09-17/TARGET-VALUES-FOR-OWNER-REVIEW.md` | fill commit | Owner-approved vs proposed |
| Architecture | `technical-package/2026-09-17/ARCHITECTURE.md` | fill commit | |
| Mechanical brief | `technical-package/2026-09-17/MECHANICAL.md` | fill commit | CAD blocker called |
| Firmware status | `technical-package/2026-09-17/FIRMWARE-STATUS.md` | fill commit | Points at PR #1 SHA above |
| Engineering BOM | `technical-package/2026-09-17/BOM.csv` | fill commit | |
| Evidence index | `technical-package/2026-09-17/EVIDENCE-INDEX.md` | fill commit | |
| Validation matrix | `technical-package/2026-09-17/VALIDATION-MATRIX.md` | fill commit | |
| BLE DFU | `technical-package/2026-09-17/BLE-DFU.md` | fill commit | NOT IMPLEMENTED on PR #1/#3 |
| Puck/app contract notes | `technical-package/2026-09-17/PUCK-APP-CONTRACT-NOTES.md` | fill commit | DFU SM missing |

After the fill commit lands, replace every “fill commit” cell with `git log -1 --format=%H -- <path>`.

## Cited sources

| Source | Repo path | Commit SHA | Used for |
| --- | --- | --- | --- |
| PR #1 firmware | `firmware/**` on `cursor/slab-visual-parity-d2a5` | `e7f7ad79c885627616819ad08114a704024569f7` | Status matrix, pins, payload stub |
| PR #1 goldens / visual spec | `design/instinct-firmware-goldens-pr1/**`, `docs/VISUAL_SPEC.md` | `e7f7ad79c885627616819ad08114a704024569f7` | UI lock; not panel proof |
| PR #3 round contract | `design/instinct-app-handoff/puck-app-round-contract-v0.1.md` | `57609af3f7e23819a89e01e5a803a8f36a80f5b8` | Import rules |
| PR #3 contract types | `apps/slab/src/lib/round-contract.ts` | `57609af3f7e23819a89e01e5a803a8f36a80f5b8` | Schema helpers; no BLE/DFU |
| PR #3 PM pressure test | `design/instinct-app-handoff/puck-app-pm-pressure-test.md` | `57609af3f7e23819a89e01e5a803a8f36a80f5b8` | Flows / non-goals |
| Sync payload (stale vs PR #3) | `docs/SYNC_PAYLOAD.md` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` | Contract tension |
| Phone app notes (stale) | `docs/PHONE_APP.md` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` | Historical only |
| P1 pin map on main (stale GIR text) | `firmware/PINMAP.md` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` | Prefer PR #1 pin map |
| One-page spec (stale) | `docs/SPEC.md` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` | Do not use steel plate / IP67 / cost claims |
| Shopping cart | `bom/shopping_cart.csv` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` | P1 list prices / URLs only |
| CAD generator | `cad/generate_enclosure.py` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` | Envelope / stack numbers |
| Enclosure bottom STEP | `cad/enclosure_bottom.step` | present on `fe7a88f` (added `91d8d84`) | Partial CAD |
| Enclosure top STEP | `cad/enclosure_top.step` | present on `fe7a88f` (added `e628ff7`) | Partial CAD |
| Magnet STEP | `cad/magnet.step` | `fe7a88f` | Concept magnet |
| Optional clamp STEP | `cad/optional_clamp.step` | `fe7a88f` | Concept clamp |
| Placeholder STLs | `cad/pcb.stl` `eink.stl` `battery.stl` `xiao.stl` `gasket.stl` | `fe7a88f` | **Placeholders** |
| Steel plate STL | `cad/steel_plate.stl` | `fe7a88f` | **Superseded** |
| **Missing** device assembly | `cad/device_assembly.step` / `.stl` | **ABSENT** | Blocker — README claims files |
| Datasheet notes | `docs/DATASHEET_NOTES.md` | `fe7a88f` | Vendor numbers; no PDFs |
| Concept renders | `concepts/web/*.jpg` | `fe7a88f` | Appearance only |
| Assembly GIF | `docs/media/assembly-preview.gif` | `fe7a88f` | Appearance only |

## Honesty reminder

Host renders / PlatformIO success ≠ hardware proof. PR #1 OTA and PR #3 DFU = **NOT IMPLEMENTED**.
