# Sync payload (phone ← device)

Device does **not** talk to GHIN. After the round (or at the turn), phone pulls this payload over BLE and may later post via a partner integration.

```json
{
  "schema": "stat-puck.round.v1",
  "round_id": "9f3c2e1a-7b84-4d2a-9c11-55aa01bf0e22",
  "device_id": "SPK-0001",
  "synced_at": "2026-09-12T18:44:00Z",
  "date": "2026-09-12",
  "course": {
    "name": "Example National",
    "tees": "Blue",
    "course_id": "local:example-national"
  },
  "holes_played": 18,
  "score_type": "Home",
  "holes": [
    {
      "hole": 1,
      "par": 4,
      "strokes": 5,
      "putts": 2,
      "fairway": "L",
      "gir": false
    }
  ]
}
```

### Field rules
- `fairway`: `H` | `L` | `R` | `NA` (forced `NA` on par 3)
- `gir`: boolean
- `score_type`: `Home` | `Away` | `Competition`
- `holes_played`: `9` or `18`
- No handicap fields on-device
