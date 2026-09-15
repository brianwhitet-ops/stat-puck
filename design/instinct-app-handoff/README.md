# Instinct app handoff (checkpoint C)

Locked Sep 15 navigation: **Bench / Rounds / Stats / Curve / Lab**.

These plates are the visual stop gate. Do not redesign the screens. The static shell in `apps/slab` is built to match them.

| File | Screen |
| --- | --- |
| `01-today.png` | Bench — home readout / next focus |
| `02-rounds.png` | Rounds — chronological record |
| `03-progress.png` | Curve — progression / trends |
| `05-stats-locked.png` | Stats — captured + derived |
| `06-lab-coach.png` | Lab — coaching / Ask |

## Product docs in this folder

- `screen-map.md` — core loop, locked tab names, Calm Field Lab palette
- `puck-app-round-contract-v0.1.md` — BLE payload, truth rules, import states
- `puck-app-pm-pressure-test.md` — save-on-device first, app-open sync, evidence-bound coaching
- `STACK.md` — proposed app stack (checkpoint C)

## Honesty locks (encode even when UI is static)

- Improvement first. Phone is post-round. Mid-round stays on the puck.
- **ROUND SAVED ON SLAB** is distinct from sync. Sync is convenience.
- v1 BLE import requires the app to be open. No background-sync promise.
- Coaching only from captured evidence (strokes, putts, drive result). GIR is derived.
- 9 or 18 holes is explicit. Power-loss resumes the open hole. Queued rounds import oldest-first. App edits keep provenance (`captured_on_slab` vs `edited_in_app`).

Concept plates still show some pre-lock copy (approach yardage, full strokes-gained columns, penalties). The shell matches those plates for visual parity. Contract comments and `apps/slab` types mark what v0.1 can actually claim.
