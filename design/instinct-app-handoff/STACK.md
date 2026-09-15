# Proposed stack — Slab app

**Vite + React + TypeScript + React Router in `apps/slab`.** Static demo now; later wrap with Capacitor (or share the contract/types with a native shell) when BLE import is real.

## Why this shape

- Lightweight web preview for visual-parity review (no store, no public deploy).
- Same React tree can move to a mobile webview without rewriting the five screens.
- Round-contract types and tests stay framework-agnostic in `apps/slab/src/lib`.
- No production backend in v1. Local durable write comes before any sync ACK.

## Preview

```bash
cd apps/slab
npm install
npm run dev
```

Open http://127.0.0.1:5173/

## Checkpoint D (scaffolded, not the stop gate)

`npm test` in `apps/slab` runs round-contract helpers (schema, GIR derivation, idempotent `round_id`, 9/18, par-3 drive null, provenance). Visual parity is checkpoint C.
