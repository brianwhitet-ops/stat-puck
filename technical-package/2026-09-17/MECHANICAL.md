# Mechanical brief (engineering input)

**Revision:** 2026-09-17  
**Source of numbers:** `cad/generate_enclosure.py` on `main` `fe7a88f100afa7f3d60271ff91e7fc2c33b08f49` unless noted. Those numbers are **concept CAD**, not inspected parts.

**Mount lock:** direct **magnet pocket** + **removable clamp**. **No steel plate in the box.** `cad/steel_plate.stl` and `make_steel_plate()` are superseded concept artifacts.

## 1. CAD inventory (honesty)

`cad/README.md` claims exports of `device_assembly.step/.stl`, internals, steel plate, and cart mount demo.

| Asset | Present on `main`? | Bytes | Label |
| --- | --- | --- | --- |
| `cad/enclosure_bottom.step` | Yes | 189294 | Concept enclosure half |
| `cad/enclosure_bottom.stl` | Yes | 361984 | Same |
| `cad/enclosure_top.step` | Yes | 113840 | Concept enclosure half |
| `cad/enclosure_top.stl` | Yes | 255684 | Same |
| `cad/magnet.step` / `.stl` | Yes | 7171 / 25084 | Simple cylinder ≈ DC6TP |
| `cad/optional_clamp.step` / `.stl` | Yes | 105974 / 83584 | Concept clamp |
| `cad/pcb.stl` | Yes | **684** | **Placeholder box** |
| `cad/eink.stl` | Yes | **684** | **Placeholder box** |
| `cad/battery.stl` | Yes | **684** | **Placeholder box** |
| `cad/xiao.stl` | Yes | **1884** | **Placeholder** |
| `cad/gasket.stl` | Yes | **1684** | **Placeholder ring** |
| `cad/steel_plate.stl` | Yes | 26284 | **Superseded — not a deliverable** |
| `cad/device_assembly.step` | **NO** | — | **BLOCKER** |
| `cad/device_assembly.stl` | **NO** | — | **BLOCKER** |
| `cad/*.step` for pcb/eink/battery/xiao/gasket/buttons | **NO** | — | Not exported |
| `full_kit_assembly` / `cart_mount_demo` | **NO** | — | Generator writes them; not in repo |
| Buttons solid | Generator-only | — | Not in repo |

**Blocker:** there is **no** complete neutral assembly STEP to send Peakingtech. This environment has **no** `build123d` (or CadQuery) module, so `python3 cad/generate_enclosure.py` cannot be run here to create the missing files. Even if regenerated, the script still emits a steel plate in `full_kit_assembly` and remains concept geometry, not production DFM.

Concept appearance only (not mechanical evidence): `concepts/web/concept-*.jpg`, `docs/media/assembly-preview.gif`.

## 2. Envelope

| Item | Concept CAD | Product target | Notes |
| --- | --- | --- | --- |
| Face | 110 × 58 mm | **Proposed** 110 × 58 mm | `W, D = 110, 58` |
| Thickness | 15.5 mm | **Proposed ≤18 mm** | Split: bottom 0.55·H, top 0.45·H |
| Corner radius | 8 mm | TBD | Outer Z fillets |
| Wall | 1.6 mm | TBD DFM | Print-oriented |
| Mass | unweighed; old spec “~95–120 g” is **not a measurement** | **Proposed ≤150 g** | |

If IP55 / drop / cell force growth: **raise envelope**, do not weaken owner-approved tests.

## 3. Display opening / module

| Item | Value | Source |
| --- | --- | --- |
| Module outline used in CAD | 89.5 × 38.0 × 1.8 mm | Script; vendor module ~89.5 × 38; panel note 36.7 × 79.0 × 1.05 mm (`docs/DATASHEET_NOTES.md`) |
| Window cut | 85.5 × 34.0 mm (`EINK_W-4` × `EINK_D-4`) | Centered at y = +8 mm on top face |
| Optical stack (lens/film/foam) | **TBD** | Needed for drop + sunlight |
| Production panel | **TBD** | Advice ask **c** |

## 4. Five-button arrangement

| Item | Value |
| --- | --- |
| Count / labels | 5: `+` `−` `PUTT` `MODE` `NEXT` (left to right in pitch order is script xs, not silk) |
| Pitch | 16.0 mm |
| Row Y | −18.5 mm (below display) |
| Top holes | Ø 8.4 mm (r = 4.2) |
| Cap solids in generator | r = 3.8 × 2.2 mm on a 6.0 × 6.6 × 4.3 mm body — **not exported** |
| Switch | Omron B3W-1000 (6 mm tactile, OF 1.57 N catalog) — **P1 selection** and production **candidate** pending DFM, wet/glove, and IP55. Not an approved production switch. |
| Glove / rain caps | **TBD** — switch IP67 ≠ product IP55 |

## 5. USB-C

| Item | Value |
| --- | --- |
| Location | Short edge, −X; cut box 8.5 × 9.2 × 3.6 mm at z ≈ 6.2 |
| P1 function | Charge + UF2 on XIAO |
| Production seal | **TBD** (cap / flap / sealed connector / service-only) — advice ask **d** |

## 6. Internal stack (concept placements)

Z values from `make_internals()` — **placeholder solids**, not a routed PCB.

| Part | Concept box (mm) | Placement (script) |
| --- | --- | --- |
| PCB | 98 × 48 × 1.2 | z = 6.0 |
| E-paper | 89.5 × 38 × 1.8 | (0, 8, 8.5) |
| XIAO | 21 × 17.5 × 5 + USB nibble | (−38, 0, 6.6) |
| Battery | 36 × 29 × 4.8 | (22, −2, 3.2) |
| Magnet | Ø19.2 × 9.6 | (0, 18, mid height + 0.2) |

**Unknowns:** real PCB outline, keep-outs, FPC, foam, EMI, service access, how a smaller cell is captured.

## 7. Magnet pocket

| Item | Value |
| --- | --- |
| Magnet | K&J DC6TP-N52, Ø19.05 × 9.52 mm, rubber-coated N52 — **P1 selection** and production **candidate** pending DFM, RF, and retention. Not an approved production magnet. |
| Pocket | Ø19.2 + 0.15 mm radius, depth MAG_H + 0.4, toward exterior bottom, at (0, 18, 0) |
| Catalog pull | **13.12 lb Case-1** (K&J) — **not** a cart or vibration test |
| Retention | Must work on ferrous carts; plastic/non-ferrous carts use the **clamp**, not a steel plate kit |

## 8. Removable clamp

| Item | Concept CAD |
| --- | --- |
| Body | 36 × 28 × 18 mm |
| Bar bore | Cylinder r = 12.5 mm (~Ø25 mm) along X |
| Older prose | Ø22–28 mm Club Car / EZ-GO (`docs/SPEC.md`) — **not modelled as a range** |
| Magnet pocket on clamp | r = MAG_D/2+0.2, h = 6, offset (0, −10, 0) |
| Slot | 3 × 10 × 20 at (12, 8, 0) — crude fastener/relief |
| Production | **TBD** (material, screw, pad, tool-less?) |

## 9. Fasteners

| Item | Concept |
| --- | --- |
| Pattern | 4× M2 bosses at (±48, ±22) |
| Bottom boss | OD 4.8, ID 1.8, height ≈ bottom split |
| Top | Through 2.2 + 4.0 × 1.2 mm cap recess (“hidden M2”) |
| Inserts vs thread-forming | **TBD** |
| Shopping-cart line | “M2 screws + inserts” — no MPN |

## 10. Gasket / sealing concept

| Item | Concept |
| --- | --- |
| Split | z = 0.55·H (≈ 8.525 mm) |
| Groove | ~0.9 mm at split; inner land |
| Gasket solid | 1.0 mm ring, `W/D − 1.8·WALL` minus inner, **placeholder STL** |
| Shopping-cart | “Silicone gasket cord 1.5 mm” — no MPN |
| Buttons / USB-C | **Not sealed in CAD** |
| Target | **IP55**, not IP67 |

Advice ask **d**.

## 11. Antenna / magnet clearance

**Not modelled.** Older `docs/RISKS.md` says “pocket opposite chip antenna.” XIAO / Raytac antenna keep-out vs N52 disc vs cart steel is an open RF problem (advice asks **b** and **e**). Do not treat concept placement as a keep-out drawing.

## 12. Materials / finish

| Item | P1 concept | Production |
| --- | --- | --- |
| Enclosure | PETG charcoal filament (shopping cart) | Injection material/finish **TBD** (older spec: matte charcoal, no toy colors, no pelican ribs) |
| Magnet coat | Rubber (K&J) | Keep non-scratch intent |
| Clamp | Printed concept | **TBD** |
| Window | Open cut | Hard-coat / film **TBD** (drop vs glass) |

## 13. Tolerance / DFM unknowns

- No 2D drawing set, GD&T, draft, shrink, or shutoff plan
- No measured printed-part fit
- Display opening vs glass/film stack
- Button travel / over-travel / wet-glove
- USB-C to split-line
- Magnet press vs adhesive vs latch
- Clamp bore tolerance vs real bars
- How IP55 and 10× 1.8 m concrete interact with a 2.9" e-paper

## 14. What to send later (not now)

When Brian releases a package to Peakingtech: this brief + whatever STEP exists, **explicitly minus** steel-plate kit language, plus a statement that internals are placeholders. Do not send `cad/README.md` uncorrected.
