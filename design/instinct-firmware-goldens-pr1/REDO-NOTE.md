# Instinct firmware visual REDO (2026-09-15 ~2:27pm ET)

Source: Instinct emails on lane.whitet@agentmail.to
- Main: Re: Brian connected us — Checkpoint verdict REDO visual layer
- Side: Slab firmware goldens 5-6 - use in PR #1

Order: Preserve behavior/state-machine; replace render layer. Check these native 296x128 PNGs into PR #1 as only visual source of truth; pixel-diff tests; no layout reinterpretation.

States:
1. 01-default-hole.png
2. 02-stroke-edit.png
3. 03-putts-input.png
4. 04-end-hole-confirm.png
5. 05-round-complete-sync.png
6. 06-derived-stats.png
+ slab-screen-states-contact-sheet.png

Next gate after emulator parity: on-device photos of all six under direct light + measured refresh.
States 5-6 UPDATED from Instinct goldens email (hashes differ from prior F-reference copies).
