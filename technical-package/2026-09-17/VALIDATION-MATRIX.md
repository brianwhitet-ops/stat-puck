# Validation matrix

**Revision:** 2026-09-17  
**Rule:** Absent evidence = **TBD**. Host goldens and `pio run` **never** count as a hardware pass.

Instinct DFU gates are included. Methods below are the intended gates for EVT / DVT, not completed tests.

| ID | Requirement | Method | Pass / fail | Evidence link | Status |
| --- | --- | --- | --- | --- | --- |
| DSP-01 | Display / real panel: 296×128 SSD1680 path; readable in sun; refresh within proposed budget | Drive P1 Waveshare (then production panel) through full + partial; photograph; log BUSY | Image matches locked layout; no ghosting that hides score; full ≤ proposed 3.0 s typical, partial ≤ 0.6 s typical **or** recorded exception | None. Host goldens on PR #1 are **not** this test. | **TBD** |
| DSP-02 | Host visual lock (software only) | `make -C firmware/host visual` on PR #1 | 0-diff at fixture values | PR #1 body claims pass @ `e7f7ad79…` — **not re-run here**; not panel proof | Host-claimed / **not hardware** |
| BLE-01 | Reliable sync at 5 m around cart; magnet **and** clamp; phone either side; max 10-round queue | Timed transfer on representative iOS + Android; record RSSI, retries, time, orientation | Pair/discover; transfer without disconnect, corruption, or duplicate; ACK only after durable app write | None | **TBD** |
| BLE-02 | 10 m unobstructed LOS: connect + one-round transfer | Same phones; LOS | Connect + one valid round; no requirement to optimize beyond 10 m | None | **TBD** |
| BLE-03 | BLE off during play | RF sniffer / phone scan during an active-round state | No advertising except setup/service | Code gate only (`slab_ble_should_advertise`) — **not RF-tested** | **TBD** (code intent I) |
| PWR-01 | Measure I_active / I_sleep / I_sync by state | Current trace: MCU, refreshes (100 partial + 20 full / 5 h), input, NVM, regulator; sleep image-hold; 5 min BLE | Honest averages for nameplate formula; e-paper hold ≈ 0 at panel | None | **TBD** |
| PWR-02 | 48 h continuous active-round | Endurance run at measured profile or integrated trace | No brownout; open/queued rounds intact | None | **TBD** |
| PWR-03 | 21-day typical use (9×5 h + 458.25 h sleep + 0.75 h sync) | Integrated trace then representative endurance | Both gates; usable ≤80% nameplate first pass | None | **TBD** |
| SAV-01 | Local save + power-loss | Pull power in DEFAULT, mid-edit, mid-persist, sleep; reboot | Resume same hole/UI; no corrupt card; no idle auto-end | LittleFS hooks only — no test log | **TBD** |
| Q-01 | Queued-round import | Load ≥10 completed unsynced rounds; app-open import oldest-first; interrupt mid-import; duplicate `round_id` | All rounds distinct; ACK after durable write; duplicate does not clone; 30-day retain (proposed) | Queue **not implemented** | **TBD** / **N** |
| BTN-01 | Buttons / debounce / wet-glove | 25 ms debounce; wet glove; rain on caps; no skip-to-putts via PUTT | Sequential lock holds; no bounce doubles; glove usable | Host unit tests ≠ wet-glove | **TBD** |
| FIT-01 | Mechanical fit | Print/machine concept enclosure + modules | Closes; window aligns; USB-C mates; buttons travel; no wire crush | No fit report | **TBD** |
| MNT-01 | Mount retention (magnet + clamp) | Representative cart ride; mixed materials; each orientation used in BLE-01 | Stays put; no paint gouge beyond rubber coat intent; clamp does not loosen | Catalog 13.12 lb ≠ this test | **TBD** |
| DRP-01 | 1.8 m / 6 ft smooth concrete; 6 faces + 4 corners = 10 drops | Assembled production-intent mass + battery; room temp; powered with saved test round | Instinct fail list (battery exposure, fire/smoke/swell, sharp edge, opens, loose magnet, detached button, unreadable cracked display, unintended input, lost/corrupt round, lost charge/BLE/button/display). Scuffs OK if those pass. | None | **TBD** |
| IP-01 | IP55 IEC 60529 | Accredited or equivalent dust + water-jet per 60529, product as sold (buttons + USB-C config) | No unsafe condition; no water on electricals that impairs function; buttons/display/charge/BLE/storage work; enclosure closed; rounds intact. **Not IP67.** | None | **TBD** |
| TMP-01 | Operating / storage temperature | Proposed 0–40 °C operate / −25–60 °C store until stack chosen | Refresh/contrast/battery/buttons still meet function; no leak | None | **TBD** (requirement proposed) |
| DFU-01 | Happy-path signed BLE update | App-download then BLE DFU; iOS + Android; version readback | New version confirmed; rounds intact | **NOT IMPLEMENTED** | **TBD** / **N** |
| DFU-02 | Interrupts | Kill app / cut BLE / reset / brown-out during transfer, erase, write, validate | Prior FW or documented recovery; **never brick** | **N** | **TBD** / **N** |
| DFU-03 | Reject bad images | Bad sig, wrong model/rev, truncated/corrupt, disallowed downgrade | Reject; no data loss | **N** | **TBD** / **N** |
| DFU-04 | First-boot fail → rollback | Plant failing image | Automatic rollback to last confirmed | **N** | **TBD** / **N** |
| DFU-05 | Data survive update | Open round + max queue bit-for-bit after success/fail/rollback | Compare hashes | **N** | **TBD** / **N** |
| DFU-06 | Repeat + schema migrate | current → new → rollback test images | Migrations safe; unknown fields ignored | **N** | **TBD** / **N** |

## Not evidence

- `concepts/web/*.jpg`, `docs/media/assembly-preview.gif`
- Instinct PNG goldens / contact sheet
- PR #1 `pio run -e xiaoble` SUCCESS
- PR #3 `npm test` / visual-parity screens
- Shopping-cart receipt / concept BOM cost
- K&J Case-1 pull figure as cart retention
- Switch IP67 as product IP55
