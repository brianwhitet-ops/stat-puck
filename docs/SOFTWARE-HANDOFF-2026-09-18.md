# Slab software handoff for scope review

Status: reviewable software implementation, not EVT validation. No communication has been sent to the manufacturer and no hardware has been purchased or ordered.

## Work supplied for review

| Area | Delivered | What the hardware partner retains |
| --- | --- | --- |
| Product behavior | Zero-start scoring, explicit capture, missing-data-aware statistics, 9/18 lifecycle, single-action NEXT handling | Real buttons, glove use, timing, display and usability integration |
| Round interface | One v2 schema, matching C/TypeScript encoders/validators and C-generated compatibility fixtures | BLE transport, pairing, GATT services, native phone integration and RF validation |
| Saved rounds | Ten-round queue, sequence identities, copy/commit ACK retirement and two-slot journal | Production NVM layout, migrations, flash wear, electrical power-loss tests and retention validation |
| App import | Bounded receiver, duplicate/conflict handling, IndexedDB transaction adapter and ACK-after-commit coordinator | Connecting these modules to the app UI/native BLE bridge and proving durability on representative phones |
| Verification | C state/protocol/fault tests, app transaction/compatibility tests and existing visual regression checks | End-to-end physical testing and defect correction |

The mobile app remains outside the manufacturer's quoted development scope. Its companion implementation supports integration and reduces overall project work; it does not by itself remove a line from their hardware fee.

## Scope that remains with the hardware partner

Production electrical architecture, component sourcing, schematic, PCB layout, antenna and magnet/cart interaction, charging and protection, production display selection, enclosure/clamp/seals, fabrication, assembly and integrated EVT validation. Signed BLE firmware update and recovery are still required and still unimplemented. No owner-approved battery, ingress, drop or RF target has been reduced.

There is no complete production CAD assembly, custom schematic or PCB supplied by this software change. Do not describe the original concept CAD as manufacturing-ready.

## Decisions made explicitly in this implementation

- Use schema v2 rather than relabeling incompatible v1 data.
- Represent unentered performance as null and use explicit capture flags on device.
- Require an explicit minus press to record zero putts. Untouched NEXT cannot fabricate a fact.
- Quick Start leaves par and yardage unknown until a course is supplied. No demo course is silently assigned.
- Use a ten-round capacity as the package's proposed implementation default, with no expiry or automatic eviction.
- Use mount-only startup rather than the SDK auto-format-on-failure routine. A fresh device requires factory/service filesystem provisioning.
- Preserve unsupported legacy files for service; no unsafe migration from prefilled scores.
- Keep the bench framework for now. Obtain vendor agreement before a production port or bootloader architecture is frozen.

## Acceptance before reducing outsourced scope

The partner should review the source, confirm which modules they will reuse, and price removed implementation separately from retained review, adaptation and integration. Source supplied is not automatically equivalent to supplier hours removed. Keep one named owner for integrated prototype behavior.

The statement of work should specify evidence for score capture, lossless queued sync, interrupted transfer, signed update/recovery and measured power, and distinguish EVT demonstrations from later DVT/certification work. Define included PCB/enclosure revision cycles and require editable source, schematics, PCB files, CAD and build instructions as deliverables.

## Validation record

- C host and packed-device logic tests: passed.
- Inactive-slot interruption tests: every one of 3,328 byte offsets passed on the host snapshot ABI.
- Invalid/corrupt slots and torn ACK retirement: passed in the injected storage model.
- App contract and import tests: 16 passed, including transaction completion before ACK, failed writes, unsupported durability, unknown schemas, duplicate import after reopen and concurrent duplicates.
- Six existing visual fixture images: pixel-identical, without golden replacement.
- App TypeScript/production build: passed.
- XIAO target build: passed, including linking and HEX/ZIP generation for `xiaoble_adafruit`. Linker-reported use is 35,236 of 237,568 bytes RAM and 146,284 of 811,008 bytes flash. This does not measure runtime stack/heap use or prove device operation.
- Build dependencies resolved to Nordic platform `10.1.0+sha.cac6fcf`, Seeed Adafruit framework `1.10101.0+sha.3ae21c3`, GCC ARM `7.2.1` and GxEPD2 `1.6.9`. Production dependency pinning remains a release-engineering task.
- Physical device, e-paper, flash durability, RF, DFU, power, ingress and drop evidence: none produced by this work.

Original package status was accurate for its pinned commits. This document describes the new work and does not alter that historical evidence record.
