# Phone app (specify only — do not ship App Store listing yet)

## Purpose
Bridge between Stat Puck and (later) GHIN. Device stays offline on-course.

## Screens (MVP)
1. **Course picker** — search course, choose tees, confirm hole count 9/18
2. **Push card** — write pars to device over BLE; show device battery
3. **Play** — status only (“recording on device”); no live score spam
4. **Pull & edit** — one-screen hole table; fix fat-finger mistakes
5. **Export / Post later** — package payload (`docs/SYNC_PAYLOAD.md`); GHIN auth placeholder

## Non-goals
- Inventing a public GHIN API
- Calculating official handicap on phone beyond what GHIN returns later
- Requiring cellular on the cart

## BLE services (draft)
- `CourseCard` — write JSON pars array
- `RoundData` — read/notify full round
- `SyncControl` — start/end sync, mark synced
- `DeviceInfo` — fw version, battery, round_id

## Score types
`Home` | `Away` | `Competition` — user picks before export.
