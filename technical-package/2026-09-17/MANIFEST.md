# Manifest — Slab technical package 2026-09-17

**Package revision:** in progress (first checkpoint). SHA column filled after each commit.  
**Base (`main`):** `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49`  
**Firmware source of truth (PR #1, not merged):** `e7f7ad79c885627616819ad08114a704024569f7`  
**App contract source (PR #3, not merged):** `57609af3f7e23819a89e01e5a803a8f36a80f5b8`  
**This branch:** `cursor/peakingtech-tech-package-0917`

Paths are repo-relative. “Package SHA” = last commit on this branch that changed the file (updated at end of fill).

## Package documents

| File | Repo path | Commit SHA | Notes |
| --- | --- | --- | --- |
| Cover note | `technical-package/2026-09-17/COVER.md` | *this branch — see git log* | Internal; not for Peakingtech yet |
| This manifest | `technical-package/2026-09-17/MANIFEST.md` | *this branch* | |
| PRD + open requirements | `technical-package/2026-09-17/PRD.md` | *this branch* | Skeleton at first commit |
| Target values | `technical-package/2026-09-17/TARGET-VALUES-FOR-OWNER-REVIEW.md` | *this branch* | Skeleton at first commit |
| Architecture | `technical-package/2026-09-17/ARCHITECTURE.md` | *this branch* | Skeleton at first commit |
| Mechanical brief | `technical-package/2026-09-17/MECHANICAL.md` | *this branch* | Skeleton at first commit |
| Firmware status | `technical-package/2026-09-17/FIRMWARE-STATUS.md` | *this branch* | Skeleton at first commit |
| Engineering BOM | `technical-package/2026-09-17/BOM.csv` | *this branch* | Skeleton at first commit |
| Evidence index | `technical-package/2026-09-17/EVIDENCE-INDEX.md` | *this branch* | Skeleton at first commit |
| Validation matrix | `technical-package/2026-09-17/VALIDATION-MATRIX.md` | *this branch* | Skeleton at first commit |
| BLE DFU requirement | `technical-package/2026-09-17/BLE-DFU.md` | *this branch* | Skeleton at first commit |
| Puck/app contract notes | `technical-package/2026-09-17/PUCK-APP-CONTRACT-NOTES.md` | *this branch* | Flags missing DFU SM |

## Cited sources (not copied unless noted)

| Source | Repo path | Commit SHA | Used for |
| --- | --- | --- | --- |
| PR #1 firmware tree | `firmware/**` on `cursor/slab-visual-parity-d2a5` | `e7f7ad79c885627616819ad08114a704024569f7` | Firmware status, pin map, payload stub |
| PR #1 goldens / visual spec | `design/instinct-firmware-goldens-pr1/**`, `docs/VISUAL_SPEC.md` | `e7f7ad79c885627616819ad08114a704024569f7` | UI states; host-render only |
| PR #3 round contract | `design/instinct-app-handoff/puck-app-round-contract-v0.1.md` | `57609af3f7e23819a89e01e5a803a8f36a80f5b8` | Import rules |
| PR #3 contract types | `apps/slab/src/lib/round-contract.ts` | `57609af3f7e23819a89e01e5a803a8f36a80f5b8` | Schema helpers; no BLE |
| PR #3 PM pressure test | `design/instinct-app-handoff/puck-app-pm-pressure-test.md` | `57609af3f7e23819a89e01e5a803a8f36a80f5b8` | Product flow / non-goals |
| Sync payload (main, stale vs PR #3) | `docs/SYNC_PAYLOAD.md` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` | Contract tension |
| Phone app notes (main, stale) | `docs/PHONE_APP.md` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` | Historical; MODE-to-GIR superseded |
| P1 pin map (main) | `firmware/PINMAP.md` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` | Wiring; PR #1 pin map supersedes GIR/MODE |
| One-page spec (main, stale) | `docs/SPEC.md` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` | Do not use steel plate / IP67 / cost claims |
| Shopping cart | `bom/shopping_cart.csv` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` | P1 vendor links / list prices only |
| CAD generator | `cad/generate_enclosure.py` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` | Envelope + stack numbers |
| Enclosure bottom STEP | `cad/enclosure_bottom.step` | `91d8d84` (added) / present on `fe7a88f` | Partial CAD |
| Enclosure top STEP | `cad/enclosure_top.step` | `e628ff7` (added) / present on `fe7a88f` | Partial CAD |
| Magnet STEP | `cad/magnet.step` | present on `fe7a88f` | Concept magnet solid |
| Optional clamp STEP | `cad/optional_clamp.step` | present on `fe7a88f` | Concept clamp |
| **Missing** device assembly | `cad/device_assembly.step` / `.stl` | **ABSENT** | Blocker — README claims files |
| Datasheet notes | `docs/DATASHEET_NOTES.md` | `fe7a88f` | Vendor numbers; no PDFs in repo |
| Concept renders | `concepts/web/*.jpg` | `fe7a88f` | Appearance only — not hardware proof |

## SHA fill rule

After the fill commit, run `git log -1 --format=%H -- <path>` for each package file and replace “*this branch*” rows. Do not invent SHAs.
