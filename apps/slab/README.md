# Slab app — Instinct checkpoint C

Static visual-parity shell for Bench / Rounds / Stats / Curve / Lab.

**Stack:** Vite + React + TypeScript + React Router in `apps/slab` (later Capacitor or a native shell for BLE).

This is a local demo. No public deploy, store listing, production backend, or hardware claims.

## Preview

```bash
cd apps/slab
npm install
npm run dev
```

Open http://127.0.0.1:5173/

## Tests (checkpoint D scaffold)

```bash
npm test
```

Round-contract helpers cover schema/checksum, derived GIR, 9/18, par-3 drive null, idempotent `round_id`, oldest-first queue, and edit provenance.

## Source

| Path | What |
| --- | --- |
| `src/screens/` | The five locked screens |
| `src/components/AppShell.tsx` | Device chrome + tab nav |
| `src/data/demo.ts` | Static plate numbers |
| `src/lib/round-contract.ts` | v0.1 payload types + import rules |
| `src/styles.css` | Calm Field Lab tokens |
