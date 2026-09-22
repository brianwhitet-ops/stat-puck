# Slab round protocol v2

Implementation proposal for review, 2026-09-18. This supersedes the two incompatible round payload scaffolds in firmware PR #1 and app PR #3 on these new branches. It does not freeze production hardware or authorize changing the owner-approved battery, ingress, drop, mount or firmware-update requirements.

## Implementation boundary

The C encoder, queue/chunk reader, TypeScript decoder, receiver and durable import coordinator are implemented and software-tested. The XIAO radio remains a stub. There is no working GATT service, native iOS/Android BLE bridge, course-card receiver or DFU implementation in this change. Do not demonstrate an app simulation as a working wireless device.

The app and firmware PRs both carry the same `protocol/` directory. Before combining them, verify those files are identical. The app re-exports the shared TypeScript types; it does not maintain a third schema.

## Round payload

`schema_version` is now **2** because nullable capture fields, sequence and checksum semantics differ from both v1 scaffolds. Unsupported schemas and extra fields are preserved and rejected without ACK. No automatic v1 conversion is attempted: the old firmware cannot distinguish real input from its prefilled scores.

| Field | Rule |
| --- | --- |
| `schema_version` | Integer 2 |
| `round_id` | Opaque ASCII identifier, 1 to 64 characters from A-Z, a-z, 0-9, colon, underscore, hyphen |
| `device_id` | Same character set, 1 to 48 characters |
| `round_sequence` | Integer 1 through 4294967295, monotonically allocated and committed before play |
| `holes_played` | Numeric 9 or 18, explicitly chosen |
| `holes` | Exactly 9 or 18 entries in ascending contiguous order, including entries with unknown values |
| `hole` | Integer index 1 through `holes_played` |
| `par` | 3, 4, 5, or null when course metadata is unavailable |
| `strokes` | Entered integer 1 through 15, or null if unentered |
| `putts` | Entered integer 0 through 10, or null if unentered; entered putts require entered strokes and cannot exceed them |
| `drive_result` | `left`, `fairway`, `right`, or null; always null on par 3 |
| `totals.strokes`, `totals.putts` | Sum only when every hole has that field entered; otherwise null, never a misleading partial total |
| `checksum` | Lowercase `crc32:xxxxxxxx`, CRC-32/ISO-HDLC over the canonical UTF-8 body excluding this field |

GIR is derived only when par, strokes and putts are known. Unknown GIR is null, not false. Fairway percentages exclude unknown drive results and unknown course pars. The approved GIR approximation is retained; no extra shot-tracking facts are invented.

Canonical property order is defined by `canonicalBody()` and the C encoder. Hole properties are ordered `hole, par, strokes, putts, drive_result`. No whitespace is emitted. Unknown timestamps and course names are not fabricated; the app can attach course metadata and its own import timestamp outside the captured payload. CRC detects corruption; it is not authentication or a firmware signature.

`fixtures/` contains actual C output for untouched holes, a nine-hole round with explicitly entered zero putts, and a completed eighteen-hole fixture. App tests check those bytes against the TypeScript canonicalizer and checksum.

## Transfer and acknowledgment

1. Transfer is allowed only after round completion, in the sync screen. All accepted score changes and completed rounds must be persisted before a saved message or radio transfer is enabled.
2. Select the oldest queued round by device sequence, not a phone clock or similarity of course/date. One round transfers at a time.
3. The transport begins a transfer with the five `RoundAck` fields below plus `byte_length` (1 to 4096). It must bind the transfer to that exact immutable round.
4. Transfer chunks carry an explicit byte offset. Chunk size follows the negotiated transport capacity. The reference receiver accepts contiguous chunks and identical overlapping retransmissions. Gaps, changed replay bytes, overflow and incomplete payloads do not produce an ACK. Reconnect may safely restart at offset zero.
5. Parse, validate, checksum and compare transfer identity. Preserve unsupported raw payloads; keep the device copy. Never partially create a round.
6. Commit the validated round in a durable atomic app transaction. `round_id` is the idempotency key. An identical duplicate updates sync metadata only; it never replaces captured data, edits, provenance or the original import time. A different payload using the same id is a conflict and gets no ACK.
7. Only after that transaction completes may the app send ACK with `schema_version, round_id, device_id, round_sequence, checksum`.
8. The device checks all ACK fields against the oldest round in the session. Copy the store, apply `slab_store_ack`, persist the copy, then publish it and send a success result. If any step fails, retain the old store and report failure. A repeated ACK for an already removed round must not remove the next round. The low-level reference returns NOT_FOUND/CONFLICT in that case; the bridge should re-enumerate the queue.
9. `sendAck()` in the app must resolve on that persisted device result. An ATT write response alone is not proof the device committed retirement. If the result is lost, reconnect and re-enumerate; any retransmitted round is an idempotent duplicate.

The production BLE adapter must implement discovery/pairing, GATT UUIDs/characteristics, fragmentation of control messages, authorization of destructive ACKs, connection/session cleanup, retry timeouts and phone permission handling. Those transport decisions remain integration work; this document does not pretend to have selected or tested them. No firmware image bytes belong in this round service.

The app's IndexedDB adapter requires a transaction that reports `strict` durability and waits for its completion event. Engines that ignore the durability option receive no ACK. This follows the [IndexedDB durability contract](https://w3c.github.io/IndexedDB/#dom-idbtransaction-durability); software tests use an in-memory IndexedDB model and cannot prove phone disk behavior. Production integration still needs device testing, storage-retention policy and backup/recovery decisions.

## Device storage

The reference store supports one active round plus ten completed rounds, with backpressure before another round starts when the queue is full. Storage is a two-slot journal with version, size, sequence and CRC checks, flushed writes and readback. An interrupted inactive-slot write preserves the previous committed snapshot. Both slots corrupt or an old unsupported `/slab_round.bin` stops normal operation for service; it never silently resets the device.

The XIAO startup calls the base mount-only filesystem operation. Its usual `InternalFS.begin()` would auto-format on mount failure and is deliberately not used. A fresh unformatted device requires an explicit factory/service format; the application will not format it automatically. The bench image for that format is `firmware/tools/provision/` (`xiaoble_provision`). It is not linked into scoring startup. The operator procedure and the silicon evidence still missing are in `firmware/tools/provision/README.md`.

Snapshots currently persist the versioned C structure layout. They are not a cross-compiler or future-firmware storage ABI. A production port must define and test migrations before changing layouts. A factory reset must preserve identity/sequence or provision a new identity namespace so old round ids cannot be reused.

These APIs use fixed workspaces and are serialized main-task APIs, not reentrant or ISR-safe. The device bridge must synchronize reads, saves, completion and ACK processing. The host file adapter is test infrastructure, not a certified database. A/B files share the underlying LittleFS medium; software fault tests do not establish real flash/brownout robustness, endurance, or the 30-day retention target.

## User interactions introduced for review

- Quick Start selects 9 or 18 holes with +/- and starts with NEXT. Unknown par/yardage remains unknown.
- Strokes begin at zero and require +/- entry. NEXT cannot promote an untouched zero into a score.
- Putts require explicit entry too. Minus at zero deliberately records a zero-putt hole; NEXT alone does not.
- Drive is initially unset. First + selects left; first - selects right; later inputs cycle L/fairway/R. Leaving it unset records null.
- At the turn, hold NEXT on untouched hole 10 to finish as nine. Holding NEXT after completion opens Quick Start for another round.
- NEXT is emitted on short release or once at the long-press threshold, never both. PUTT remains physically unchanged and ignored as in the approved sequential flow.

The original six visual fixture goldens remain unchanged. The new Quick Start, unentered fields and device storage-error screens need owner usability review and real-panel testing.
