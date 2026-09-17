# Production BLE DFU requirement

**Revision:** 2026-09-17  
**Owner requirement** (Instinct / Brian). Applies to **production** Slab units.

## Implementation status (do not blur)

| Surface | Status |
| --- | --- |
| PR #1 firmware `e7f7ad79c885627616819ad08114a704024569f7` | **OTA / DFU NOT IMPLEMENTED.** No bootloader, no DFU service, no signed image pipeline. nRF52840 BLE ≠ OTA. |
| PR #3 app `57609af3f7e23819a89e01e5a803a8f36a80f5b8` | **DFU state machine NOT IMPLEMENTED.** Round-contract types have no update states. Visual shell only. |
| P1 USB-C / UF2 on XIAO | Factory/dev flash only. Not the golfer production path. |
| This package | Requirement + validation gates. Architecture **TBD** (Peakingtech advice ask **f**). |

## Why it exists

Later features and data-collection changes must ship **without physical access** to every puck.

## Product path

1. Authenticated firmware download **to the Slab app**.
2. App-to-puck device firmware update **over BLE**.
3. **Do not** overload the round-data GATT / v0.1 payload as an untyped blob.
4. Define a **separate versioned DFU/update service and state machine** beside the round-sync protocol.

Updates **never begin during an active round**.

## Image authenticity / integrity

- Images are **cryptographically signed**.
- Bootloader verifies **signature and image hash** before activation.
- Reject: wrong device / hardware revision, downgrade outside policy, corrupt, unsigned, or incomplete images.

## Brick protection

- Prefer **dual-bank / A/B** (or swap) with **confirmed boot** and **automatic rollback**.
- If flash/RAM cannot do that, specify a **protected immutable recovery bootloader** and a **resumable** recovery path.
- Application failure must **not** remove the ability to update or recover.

## Power / interruption safety

- App and puck check **battery level** before start.
- Update is **resumable or safely restartable** after phone disconnect, BLE loss, or puck reset.
- **Never erase the only bootable image** before the replacement is validated.
- All saved / open / queued round data stay intact through success, failure, and rollback.

## Product states (app + puck)

Must exist as explicit UI/device states:

- available update
- downloading to phone
- puck preflight
- transferring (progress)
- validating
- rebooting
- confirming
- complete

Recoverable states:

- interrupted
- rejected
- rolled-back

## Versioning

`DeviceInfo` (not implemented) must expose:

- bootloader version
- firmware version
- hardware revision
- schema compatibility

Preserve migration rules for stored rounds and BLE payload schemas. Unknown future fields must not corrupt or discard existing data.

## Security lifecycle (TBD owners — do not invent)

Must be defined before production:

- signing-key ownership and storage
- manufacturing provisioning
- release authorization
- downgrade policy
- vulnerability / update support window
- lost / compromised-key recovery

**No** engineering backdoor or unsigned production mode.

## Manufacturing / service

- Accessible **SWD / debug pads** for factory program and recovery, **locked** appropriately in production
- Unique device identity where needed (today: hardcoded `SPK-0001`)
- Factory image and production test record traceable by serial / hardware revision

## Validation gates (all TBD — no evidence)

| Gate | Pass |
| --- | --- |
| Happy path | Successful update + version readback on representative **iOS and Android** |
| Interrupt | Kill app, cut BLE, reset / brown-out puck at transfer / erase / write / validate; every case returns to prior firmware, resumes safely, or enters **documented** recovery — **never bricks** |
| Reject | Bad signature, wrong model/revision, corrupted/truncated image, disallowed downgrade — rejected, **no data loss** |
| Rollback | New-image first boot fails on purpose; automatic rollback restores last confirmed image |
| Data | Open round + **maximum queued-round** data survive update / failure / rollback **bit-for-bit** |
| Repeat | Current → new → rollback test images, including storage-schema migrations |

## Advice ask for Peakingtech (not authority)

For the selected MCU/module and memory budget, recommend:

- Nordic **Secure DFU** / **MCUboot** vs another **proven signed** path
- Dual-bank vs recovery-mode tradeoff
- Flash / storage overhead (image slots + swap + logs vs nRF52840 1 MB / XIAO vs Raytac)
- Bootloader / signing / provisioning **ownership**
- App integration (iOS/Android DFU libraries vs custom GATT)

P1 XIAO uses Adafruit nRF52 / UF2. Production Raytac MDBT50Q-1MV2 is a **candidate**. Do not assume SoftDevice DFU is “already there.”

## Contract flag

See `PUCK-APP-CONTRACT-NOTES.md`: import states exist on paper; **DFU states do not**.
