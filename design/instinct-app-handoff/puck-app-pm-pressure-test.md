# Slab puck -> app: opinionated golfer flow

## The product promise
Slab should save the round even if the phone never leaves the bag. The app makes the round useful later. Sync is a convenience layer, never a condition for scoring.

## Strongest end-to-end flow

### 1. Before the first round: pair once
The one-time setup pairs the puck to the account and names the device. That is the only pairing ceremony a golfer should feel.

For each round, the default is **Start round on the puck**. If the app is available, it may preload course, tees, par and hole count. If it is not, Quick Start creates a round locally with hole numbers and par defaults that can be corrected. Do not make a foursome wait while someone searches for a course or fixes Bluetooth.

PM line: course selection is helpful metadata, not an entry ticket.

### 2. On the first tee: under 20 seconds
The puck shows course/tee if preloaded, 18 or 9 holes, and Start. If no course was loaded, it shows Quick Start. A long press prevents accidental starts.

No login, phone unlock, GPS lock or network dependency on the tee.

### 3. During play: the phone disappears
The puck owns strokes, putts and drive result on par 4/5. One glance shows hole, par and current score. The user can correct the current or just-finished hole without opening the app.

Do not ask for GIR, penalties, club or distance. GIR comes from score and putts. Each extra prompt costs pace and makes Slab feel like homework.

A rain delay is not a special mode. The puck simply keeps the round open through sleep and power loss. Resume returns to the same hole. A deliberate End round prevents an idle timer from killing a slow round.

### 4. Nine holes and unusual rounds: explicit, not guessed
At start, choose 9 or 18 when known. At the turn, an 18-hole round can End now as 9. Never auto-finish because the golfer has been inactive or because nine holes were entered.

For a shotgun start, the initial hole can change. For a replayed hole or playoff, v1 should end the regulation round and avoid inventing unsupported scorecard structures.

### 5. Walking off 18: save first, sync second
After the final hole, the puck shows a short review: total score, total putts and any hole with incomplete drive result. The golfer can:
- Finish and save
- Go back one hole

The success screen must say **ROUND SAVED ON SLAB** before it says anything about the phone. That is the trust moment. Only then does the puck enter sync-ready mode.

Do not hold the group at the green for Bluetooth. Sync can happen in the cart, parking lot, clubhouse or days later.

### 6. Sync: app-open is the honest v1
Mobile background Bluetooth behavior is too variable to make invisible sync the promise. In v1, opening the app shows a clear puck card:
- Slab found
- 1 round waiting
- Importing
- Saved

The app ACKs only after a durable, validated write. The puck keeps every unacknowledged round. A duplicate is ignored by round ID. If the phone dies or Bluetooth drops, nothing changes on the puck except “Not synced.”

The puck should hold at least a long weekend of completed rounds. The app imports the queue oldest-first and shows each round separately. Never merge Saturday and Sunday because course/date metadata is similar.

### 7. Parking-lot debrief: reward, do not detain
After import, the app opens a compact Round Debrief:
- score and putts
- fairways and miss direction
- GIR derived from score and putts
- one factual “what changed” comparison against the golfer’s recent baseline

Buttons: Review holes / Done. No forced questionnaire and no instant coaching essay. The golfer may be driving, packing clubs or joining friends.

### 8. Coaching later: humble, evidence-bound
Lab turns repeated patterns into a one- or two-week experiment. With the current inputs, it can coach:
- tee-shot accuracy and left/right tendency
- GIR trend
- putting volume and three-putt proxies only if hole-level putts support them
- scoring by par and late-round patterns
- conversion after missed greens only where derivable

It cannot honestly coach “125-175 yard approaches,” club selection, proximity, penalties or full strokes gained without more inputs. The current concept’s “Own 125-175 yards” example should be removed. Over-specific coaching would break trust faster than a sync failure.

A good coach says what it knows, why it thinks it, and what result would change its mind.

## Edge-case decisions

| Situation | Product behavior |
|---|---|
| Phone dead all round | Nothing changes. Round saves locally and syncs later. |
| Forgot to sync for days | Puck badge shows queued-round count; next app open imports all, oldest-first. |
| Multiple rounds on a trip | Unique round IDs, local start order, never overwrite; course can be added later. |
| Rain delay / six-hour round | Round survives sleep and restart; no idle auto-end. |
| Nine-hole round | Explicit 9-hole start or End now at the turn; no auto-finish. |
| Partner waiting | Quick Start and no required phone/course search. Final-hole save takes seconds; sync waits. |
| App edits a score later | Correction is allowed, but preserve “captured on Slab” vs “edited in app” provenance. Do not silently rewrite puck history. |
| Puck reconnects after an interrupted import | Resume/retry whole immutable payload; idempotent app write; ACK after validation. |
| Unknown app schema | Keep the round on puck, tell user app update is needed, never discard. |

## What earns trust
- “Saved on Slab” is distinct from “Synced to phone.”
- No round depends on network, GPS or phone battery.
- The app never duplicates or merges rounds.
- Coaching stays inside the evidence actually captured.
- Setup and finish never make playing partners wait.

## What will annoy golfers
- Mandatory course search on the first tee.
- A Bluetooth spinner before the round counts as saved.
- Extra inputs marketed as “better stats.”
- Forced post-round review before the round is safely stored.
- Coaching that sounds precise but relies on data Slab never collected.
- Quietly losing a Saturday round because Sunday’s import reused its identity.

## Product cuts for v1
Cut background-sync promises, live phone dashboards, shot distances, clubs, penalties, full strokes gained, social feeds and automatic course detection. Nail local save, queued import, a factual debrief and humble Lab coaching first.
