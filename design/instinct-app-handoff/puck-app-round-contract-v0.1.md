# Slab puck -> app round contract v0.1

## Product boundary
The puck stays the in-round source of truth. The phone app imports only after the golfer confirms the final hole and the puck enters ROUND_COMPLETE_SYNC. No live-round phone dependency.

## Transport
- BLE transfer initiated by the app after pairing.
- Puck advertises only in ROUND_COMPLETE_SYNC.
- App requests completed rounds not yet acknowledged by that app/account.
- Each payload has schema_version, round_id, device_id, sequence and checksum.
- App validates, writes idempotently by round_id, then sends an ACK.
- Puck retains an unacknowledged round for retry after disconnect/power cycle. Never delete on transmit alone.
- Duplicate transfers update sync metadata, not duplicate the round.

## Export payload
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
  "totals": {"strokes": 84, "putts": 33},
  "checksum": "..."
}
```

## Truth rules
- drive_result is null on par 3; required on par 4/5 only if the player supplied it.
- GIR is derived by the app from strokes - putts <= par - 2. It is not a dedicated puck input.
- score, putts and drive result are captured. Round/hole totals and GIR are derived.
- Penalties, shot distances, proximity, club selection and full strokes-gained categories are not in v0.1 because the current puck flow does not capture them.
- Course name and hole yardage can be attached later in the app, but may not rewrite captured fields.

## App import states
Not paired -> Pairing -> Connected -> Importing x/y -> Imported -> Acknowledged.
On validation/checksum failure: preserve payload, show Retry, do not partially create a round.
On unknown schema: preserve payload and require app update; do not discard.
