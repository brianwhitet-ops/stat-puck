# Manifest — Slab technical package 2026-09-17

**Package revision date:** 2026-09-17  
**Branch:** `cursor/peakingtech-tech-package-0917`  
**PR:** https://github.com/brianwhitet-ops/stat-puck/pull/4  
**Base `main`:** `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49`  
**First checkpoint (skeleton + cover):** `bc0c64d02ad60da5a0f7e064ba86e616a12139c3`  
**Fill commit (complete docs):** `d88e7a3e1d551c42cc5706875de540de6f04e99b`  
**Reviewed lock (Instinct file-by-file):** `03dd4047d4915095430c125e703db08d5fa5361e`  
**Package content SHA:** `56aa033e4d184133fd6fbc700535eb44af615c6e`  
**Firmware (PR #1, not merged):** `e7f7ad79c885627616819ad08114a704024569f7`  
**App contract (PR #3, not merged):** `57609af3f7e23819a89e01e5a803a8f36a80f5b8`

## SHA scheme (non-self-referential)

A git commit cannot contain its own hash. This file therefore does **not** use a field named “This manifest lock.”

| Name | Meaning | Verify |
| --- | --- | --- |
| **Package content SHA** | Last commit that changed any file under `technical-package/2026-09-17/` **except** this lock document. Today that is the Instinct four-item correction. | `git log -1 --format=%H -- technical-package/2026-09-17 ':!technical-package/2026-09-17/MANIFEST.md'` |
| **Lock document** | This file at branch HEAD. That SHA is **not written below**. | `git rev-parse HEAD` must equal `git log -1 --format=%H -- technical-package/2026-09-17/MANIFEST.md` |
| **Package-document row SHA** | Last commit that changed **that path**. | `git log -1 --format=%H -- <path>` |

Do not treat Package content SHA as the commit that contains the current `MANIFEST.md`.

Paths are repo-relative.

## Package documents

| File | Repo path | Commit SHA | Notes |
| --- | --- | --- | --- |
| Cover | `technical-package/2026-09-17/COVER.md` | `56aa033e4d184133fd6fbc700535eb44af615c6e` | Internal; not for Peakingtech yet; ENT-01 firmware-conflict blocker |
| This manifest | `technical-package/2026-09-17/MANIFEST.md` | HEAD (not stored; see SHA scheme) | Lock document. Content this map describes: `56aa033e4d184133fd6fbc700535eb44af615c6e`. |
| PRD + open requirements | `technical-package/2026-09-17/PRD.md` | `56aa033e4d184133fd6fbc700535eb44af615c6e` | Zero-start hole entry (ENT-01) |
| Target values | `technical-package/2026-09-17/TARGET-VALUES-FOR-OWNER-REVIEW.md` | `56aa033e4d184133fd6fbc700535eb44af615c6e` | Owner-approved vs proposed; ENT-01 |
| Architecture | `technical-package/2026-09-17/ARCHITECTURE.md` | `56aa033e4d184133fd6fbc700535eb44af615c6e` | Zero-start + BOM status wording |
| Mechanical brief | `technical-package/2026-09-17/MECHANICAL.md` | `56aa033e4d184133fd6fbc700535eb44af615c6e` | CAD blocker called; B3W / K&J candidate wording |
| Firmware status | `technical-package/2026-09-17/FIRMWARE-STATUS.md` | `56aa033e4d184133fd6fbc700535eb44af615c6e` | Points at PR #1 SHA; hole-default conflict |
| Engineering BOM | `technical-package/2026-09-17/BOM.csv` | `56aa033e4d184133fd6fbc700535eb44af615c6e` | Production-status correction |
| Evidence index | `technical-package/2026-09-17/EVIDENCE-INDEX.md` | `d88e7a3e1d551c42cc5706875de540de6f04e99b` | Unchanged this revision |
| Validation matrix | `technical-package/2026-09-17/VALIDATION-MATRIX.md` | `56aa033e4d184133fd6fbc700535eb44af615c6e` | STAT-01 / STAT-02 |
| BLE DFU | `technical-package/2026-09-17/BLE-DFU.md` | `d88e7a3e1d551c42cc5706875de540de6f04e99b` | NOT IMPLEMENTED on PR #1/#3; unchanged this revision |
| Puck/app contract notes | `technical-package/2026-09-17/PUCK-APP-CONTRACT-NOTES.md` | `56aa033e4d184133fd6fbc700535eb44af615c6e` | DFU SM missing; par vs performance |

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
| Magnet STEP | `cad/magnet.step` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` | Concept magnet |
| Optional clamp STEP | `cad/optional_clamp.step` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` | Concept clamp |
| Placeholder STLs | `cad/pcb.stl` `eink.stl` `battery.stl` `xiao.stl` `gasket.stl` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` | **Placeholders** |
| Steel plate STL | `cad/steel_plate.stl` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` | **Superseded** |
| **Missing** device assembly | `cad/device_assembly.step` / `.stl` | **ABSENT** | Blocker — README claims files |
| Datasheet notes | `docs/DATASHEET_NOTES.md` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` | Vendor numbers; no PDFs |
| Concept renders | `concepts/web/*.jpg` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` | Appearance only |
| Assembly GIF | `docs/media/assembly-preview.gif` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` | Appearance only |

## Honesty reminder

Host renders / PlatformIO success ≠ hardware proof. PR #1 OTA and PR #3 DFU = **NOT IMPLEMENTED**.
