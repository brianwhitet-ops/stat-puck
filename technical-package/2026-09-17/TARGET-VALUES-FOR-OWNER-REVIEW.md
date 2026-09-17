# Target values for owner review

**Revision:** 2026-09-17  
**Rule:** Owner-approved rows are **requirements**, not placeholders. Other rows are **proposed** until the owner-decision column is filled. Do not treat recommendations as approved.

Peakingtech may advise; they do not choose or spend.

| Requirement | Recommended value / range | Rationale (1 sentence) | Status | Owner decision |
| --- | --- | --- | --- | --- |
| Battery — active-round endurance | ≥48 h continuous active-round operation per charge | Owner-set outcome so a long day plus a second round cannot brown out mid-card. | **Owner-approved** | |
| Battery — typical-use endurance | ≥21 calendar days; 3 rounds/week × 3 weeks = 9 rounds / 45 active hours | Owner-set “about three weeks” translated to a countable duty cycle. | **Owner-approved** | |
| Battery — duty cycle (active) | 5 h / 18-hole round; e-paper **hold ≈ 0**; 100 partial + 20 full refreshes/round; buttons/NVM for 18 holes; **BLE off** in play | Matches an unhurried round and the product rule that the panel costs energy only when it refreshes. | **Owner-approved** | |
| Battery — duty cycle (21-day rest) | Sleep **458.25 h** in the 504 h window; 9 × 5 min BLE sync = **0.75 h** | Sync sessions sit inside the 21-day window (504 − 45 − 0.75 = 458.25). | **Owner-approved** | |
| Battery — first-pass usable | ≤80% of nameplate; then add a **separate** measured engineering margin (do not double-count) | Covers aging / cutoff / conversion on paper before EVT numbers exist. | **Owner-approved** | |
| Battery — nameplate formula | `max(48h×I_active, 45h×I_active + 458.25h×I_sleep + 0.75h×I_sync) / 0.80` | Converts measured (or honest modelled) currents into a minimum nameplate, not a preferred SKU. | **Owner-approved** | |
| Battery — 500 mAh | P1 **starting assumption only** (Adafruit 1578). **Not a floor.** Size down if both gates pass. | Thickness, mass, and cost should follow the budget, not the concept cell. | **Owner-approved** | |
| Battery — candidate set | Compare **150 / 200 / 250 / 300 / 500 mAh** on capacity, thickness, footprint, mass, charge time, sourceability, protection, margin | Owner-required comparison before any cell is selected. | **Owner-approved** (comparison) | |
| Water / ingress | **IP55** per IEC 60529; rain/splash/jets; **not IP67 / not submersion** | Outdoor cart use without pretending the puck is a dive watch. | **Owner-approved** | |
| Drop height / surface | **1.8 m / 6 ft** onto **smooth concrete** | Higher than a typical cart mount so a knock-off still has a defined test. | **Owner-approved** | |
| Drop sequence | 6 faces + 4 most-vulnerable corners = **10 drops**; powered/active with a saved test round; one room-temp sample (fresh sample only if Peakingtech shows cumulative damage hides the limit) | Instinct-specified sequence; cosmetics may scuff if safety/function pass. | **Owner-approved** | |
| Drop pass / fail | Fail if: battery exposure, fire/smoke/swelling, sharp edge, enclosure opening, loose magnet, detached button, cracked display that impairs read, unintended input, loss/corruption of open or saved round, or loss of charge/BLE/button/display. Scuffs/dents OK if those pass. | Instinct-specified safety + function bar. | **Owner-approved** | |
| BLE — play | **Off** during play except deliberate setup/service | Saves energy and matches “phone in the bag.” | **Owner-approved** | |
| BLE — cart range | **Reliable sync at 5 m around the cart** (magnet **and** clamp orientations; phone either side of cart; max **10-round** queue; no disconnect / corruption / duplicate; ACK only after durable app write) | Real parking-lot / cart-path geometry, not a lab coax number. | **Owner-approved** | |
| BLE — verification | **10 m unobstructed LOS**: basic connect + one-round transfer; do not optimize beyond 10 m | Caps RF work; records RSSI/retries/time/orientation on representative iOS + Android. | **Owner-approved** | |
| BLE DFU | Signed images, brick protection, interrupt-safe, separate GATT, production lifecycle — see `BLE-DFU.md` | Later features cannot require a USB cable on every puck. | **Owner-approved requirement; not implemented** | |
| Display full refresh | **Proposed:** ≤3.0 s typical (Waveshare catalog ~3 s; PR #1 `SLAB_REFRESH_FULL_MS 3000`) | Budget the 20 full refreshes/round until a production panel is measured. | **Proposed** | |
| Display partial refresh | **Proposed:** ≤0.6 s typical (catalog ~0.6 s; PR #1 `SLAB_REFRESH_PARTIAL_MS 600`, typ 300) | Budget the 100 partials/round; not a certified spec. | **Proposed** | |
| Operating temperature | **Proposed:** 0–40 °C (golfer-facing, e-paper-class) | Golf is outdoor; e-paper contrast/refresh is the usual limiter — **untested**. | **Proposed** | |
| Storage temperature | **Proposed:** −25–60 °C (cell + panel vendor windows TBD) | Car-trunk and shipping; not a claim until cell/panel stack is chosen. | **Proposed** | |
| Mount vibration / retention | **Proposed:** stay on cart (magnet **and** clamp) through a representative 18-hole cart ride + remain safe after DRP-01; no invented g RMS | Mixed materials and vibration are the actual mount problem; we have no vibe profile yet. | **Proposed** | |
| Envelope — face | **Proposed:** 110 × 58 mm (current CAD) | Matches the concept slab and 2.9" landscape module. | **Proposed** | |
| Envelope — thickness | **Proposed:** ≤18 mm (CAD concept is 15.5 mm) | Leaves a few mm if IP55 / drop / cell force growth; **do not shrink BAT/DRP/IP to protect 15.5**. | **Proposed** | |
| Envelope — mass | **Proposed:** ≤150 g (older spec “~95–120 g” is an **unweighed estimate** — discarded as a claim) | Pocketable / cart-safe; no scale data. | **Proposed** | |
| Button durability | **Proposed:** wet-glove usable; life = selected switch rating after cap/DFM (Omron B3W-1000 catalog: sealed switch, OF 1.57 N — **product life untested**) | Five large pads beat a touchscreen in rain; do not invent a cycle number beyond the switch datasheet. | **Proposed** | |
| Unsynced retention | **Proposed:** ≥10 completed rounds **and** ≥30 days without import | Covers a trip plus a forgotten Sunday sync. | **Proposed** | |
| Launch compliance design-in | **Proposed:** US + EU (FCC + CE/RED). **Not certified. No pre-scan.** | Matches intended first markets; UKCA/RoHS/UN38.3 are follow-ons, not claims. | **Proposed** | |
| Charge time | **TBD** — no measured charge current or selected charger IC for production | XIAO BQ25101 exists on P1; hours must not be invented. | **TBD** | |

## Illustrative nameplate math (not a cell selection)

Using **illustrative** I_sleep = 0.1 mA and I_sync = 8 mA **only** (owner example; **not measured**):

| I_active (mA) | 48 h energy (mAh) | 21-day energy (mAh) | max / 0.80 → nameplate (mAh) |
| --- | --- | --- | --- |
| 1.0 | 48 | 45 + 45.825 + 6 = 96.825 | **121** |
| 1.5 | 72 | 67.5 + 45.825 + 6 = 119.325 | **149** |
| 2.0 | 96 | 90 + 45.825 + 6 = 141.825 | **177** |
| 2.5 | 120 | 112.5 + 45.825 + 6 = 164.325 | **205** |
| 3.0 | 144 | 135 + 45.825 + 6 = 186.825 | **234** |
| 5.0 (old ceiling, not a design target) | 240 | 225 + 45.825 + 6 = **276.825** | **346** |

21-day mix at those illustrative sleep/sync currents: `45×I_active + 45.825 + 6`.

**Candidate first-pass usable (80%):** 150→120, 200→160, 250→200, 300→240, 500→400 mAh.  
At 1.5 mA illustrative, 150 mAh is on the paper edge; at 2 mA, 200 mAh is the first listed candidate that clears ~177. **Do not select** until I_active / I_sleep / I_sync are measured and a commercially suitable **protected** cell exists.

Catalog examples used for size/mass comparison (vendor-published, not Slab measurements):

| Nameplate | Example SKU | Vendor size / mass | Notes |
| --- | --- | --- | --- |
| 150 mAh | Adafruit 1317 | 19.75 × 26.02 × 3.8 mm; 4.65 g | Protected pouch + JST-PH; P1-class only |
| 200 mAh | — | **TBD** | No selected MPN this revision |
| 250 mAh | — | **TBD** | No selected MPN this revision |
| 300 mAh | — | **TBD** | No selected MPN this revision |
| 350 mAh (nearby, not required) | Adafruit 2750 | 36 × 19.6 × 5.2 mm; 8.2 g | Thicker than 500 mAh 1578 — stack risk |
| 500 mAh | Adafruit 1578 | 29 × 36 × 4.75 mm; 10.5 g | Current CAD battery box |

Charge time, UN38.3 pack docs, and production-cell sourceability: **TBD** (Peakingtech advice ask a).

## Conflicts to surface, not hide

- If measured I_active is high, **grow the cell or cut refresh energy** — do not redefine “active” as sleep.
- If IP55 + 1.8 m drop + magnet pocket need more than 18 mm or 150 g, **flag the envelope**, do not quietly drop IP/drop/battery.
- 500 mAh must not be kept “because the concept used it.”
