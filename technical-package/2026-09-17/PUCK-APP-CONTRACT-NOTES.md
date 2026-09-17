# Puck ↔ app contract notes (v0.1) + DFU gap

**Revision:** 2026-09-17  
**Sources (not on `main`):**

| File | Commit |
| --- | --- |
| `design/instinct-app-handoff/puck-app-round-contract-v0.1.md` | `57609af3f7e23819a89e01e5a803a8f36a80f5b8` (PR #3) |
| `apps/slab/src/lib/round-contract.ts` | same |
| `design/instinct-app-handoff/puck-app-pm-pressure-test.md` | same |
| `firmware/src/slab_ble.c` payload builder | `e7f7ad79c885627616819ad08114a704024569f7` (PR #1) |
| `docs/SYNC_PAYLOAD.md` | `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` (`main`, stale) |

Website and PR #3 screens define **rules and types**, not a working radio.

## Product boundary

- Puck is the in-round source of truth.
- Phone imports **after** the golfer finishes and the puck is in `ROUND_COMPLETE_SYNC`.
- No live-round phone dependency. v1 import is **app-open**, not background sync.
- ACK **only** after a durable, validated write. Puck keeps unacknowledged rounds across disconnect and power cycle. Never delete on transmit alone.
- Idempotent by `round_id`. Duplicates update sync metadata only.
- GIR is **derived** (`strokes − putts ≤ par − 2`). Not a dedicated puck input. No MODE-to-GIR.
- Drive is null on par 3; on par 4/5 only if the golfer set it.
- 9 vs 18 is explicit (start as 9, or End-now-as-9). Never infer from idle.
- App edits keep provenance (`captured_on_slab` vs `edited_in_app`).
- Unknown schema: preserve payload, require app update, do not discard.

## PR #3 payload (contract)

```json
{
  "schema_version": 1,
  "round_id": "uuid",
  "device_id": "stable-device-id",
  "started_at": "optional-local-timestamp",
  "completed_at": "optional-local-timestamp",
  "holes_played": 18,
  "holes": [
    {
      "hole": 1,
      "par": 4,
      "strokes": 5,
      "putts": 2,
      "drive_result": "left|fairway|right|null"
    }
  ],
  "totals": { "strokes": 84, "putts": 33 },
  "checksum": "..."
}
```

Import states in `round-contract.ts`:  
`not_paired` → `pairing` → `connected` → `importing` → `imported` → `acknowledged`, plus `retry` and `app_update_required`.

Helpers cover checksum, derived GIR, 9/18, par-3 drive null, idempotent upsert, oldest-first queue, provenance. **`npm test` is a scaffold, not BLE.**

## PR #1 builder (what the puck can emit today)

`slab_ble_build_payload` emits `{"schema":"stat-puck.round.v1",...}` with `fairway` `L|H|R|NA`, device-computed `"gir":true|false`, hardcoded `course_id: local:example`, `score_type: Home`, **no** checksum/sequence/totals/timestamps. Device id is `"SPK-0001"`. Round id on demo init is a fixed UUID.

**These two schemas are not compatible.** Unification is an open requirement (`CON-01`). This package does not pick a winner.

## DFU state machine — NOT PRESENT

PR #3 contract and `ImportState` cover **round import only**.

There is **no** DFU/update service, characteristic set, or state enum for:

- available update
- downloading to phone
- puck preflight
- transferring (progress)
- validating / rebooting / confirming / complete
- interrupted / rejected / rolled-back

There is **no** DeviceInfo surface for bootloader / firmware / hardware revision / schema compatibility.

Until that exists in the contract **and** firmware:

- Mark DFU **NOT IMPLEMENTED**
- Do not imply OTA from BLE capability
- Do not stuff images through the round payload

Required behavior: `BLE-DFU.md`.

## What is implemented vs not

| Piece | Status |
| --- | --- |
| Written v0.1 rules + TS helpers + unit tests (PR #3) | Implemented as **scaffold** |
| BLE scan / pair / GATT / import UI | **Not implemented** |
| Device GATT + ACK | **Not implemented** |
| Course-card write | **Not implemented** |
| Multi-round device queue | **Not implemented** |
| DFU SM (app + puck) | **NOT IMPLEMENTED** |

## Stale `main` notes

`docs/PHONE_APP.md` still lists CourseCard / RoundData / SyncControl and an older GHIN-shaped story. Use this file + PR #3 for review. Do not send `PHONE_APP.md` to Peakingtech as current.
