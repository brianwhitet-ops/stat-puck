/**
 * Static demo data matching design/instinct-app-handoff goldens.
 *
 * Visual parity uses these numbers. v0.1 captured fields are strokes, putts,
 * and drive result. GIR is derived. Course name is app metadata.
 *
 * Not in the puck payload (do not coach as fact later):
 * approach yardage, penalties, full strokes-gained columns, club, proximity.
 *
 * Import / queue (not drawn on the locked plates — app-open v1):
 * Not paired → Pairing → Connected → Importing x/y → Imported → Acknowledged.
 * Power-loss resume lives on the puck; the app never owns an in-round card.
 */

export const bench = {
  eyebrow: "Slab · Field Lab",
  title: "Bench",
  dek: "A clear readout on where your game is going.",
  windowLabel: "Last 8 rounds",
  handicap: "12.4",
  handicapDelta: "2.1 since May",
  sparkline: [8, 11, 10, 14, 16, 15, 18, 22],
  /**
   * Plate copy for visual parity (01-today.png).
   * PM pressure test: do not later coach 125–175 yd approaches — the puck
   * does not capture proximity or club. Evidence-bound rewrite is checkpoint D+.
   */
  opportunityKicker: "Approach · 125–175 yd",
  opportunityValue: "6.2",
  opportunityUnit: "shots / round",
  opportunityNote: "Lost vs. your 10-handicap target",
  practiceTitle: "Distance control · 35 min",
  practiceMeta: "3 landing zones · 8 balls each",
  practiceWhy: "Why: short misses rose in 3 straight rounds",
};

export const rounds = [
  {
    id: "2026-09-14-common-ground",
    date: "Sep 14",
    course: "Common Ground",
    score: 82,
    sg: "+3.4",
    putts: 31,
    gir: 8,
    holesPlayed: 18 as const,
    provenance: "captured_on_slab" as const,
  },
  {
    id: "2026-09-07-lone-tree",
    date: "Sep 07",
    course: "Lone Tree",
    score: 79,
    sg: "+0.8",
    putts: 31,
    gir: 8,
    holesPlayed: 18 as const,
    provenance: "captured_on_slab" as const,
  },
  {
    id: "2026-08-30-arrowhead",
    date: "Aug 30",
    course: "Arrowhead",
    score: 85,
    sg: "+5.7",
    putts: 31,
    gir: 8,
    holesPlayed: 18 as const,
    provenance: "captured_on_slab" as const,
  },
  {
    id: "2026-08-23-fossil-trace",
    date: "Aug 23",
    course: "Fossil Trace",
    score: 81,
    sg: "+2.6",
    putts: 31,
    gir: 8,
    holesPlayed: 18 as const,
    provenance: "captured_on_slab" as const,
  },
];

export const stats = {
  filters: ["Last 10", "All courses", "Vs. 10 hcp"] as const,
  scoring: [
    { label: "Avg score", value: "81.7" },
    { label: "To par", value: "+9.7" },
    { label: "Penalties", value: "1.2" },
  ],
  fairways: { value: "54%", miss: "L 22 · R 24" },
  gir: { value: "44%" },
  putts: { value: "31.4", unit: "/ round" },
  upAndDown: { value: "38%" },
  /** Plate columns for parity. v0.1 cannot honestly claim full SG categories. */
  strokesGained: [
    { label: "Off tee", value: "−0.8" },
    { label: "Approach", value: "−2.6" },
    { label: "Short", value: "+0.2" },
    { label: "Putt", value: "−0.3" },
  ],
};

export const curve = {
  metrics: ["Strokes gained", "Handicap"] as const,
  value: "−2.4",
  unit: "shots / round",
  note: "better over 12 rounds",
  series: [10, 8, 6, 11, 12, 16, 18, 20],
  trend: [7, 9, 11, 13, 15, 17, 19, 21],
  changedTitle: "Putting is carrying the improvement.",
  changedLines: ["−1.3 putts / round", "3-putts down 41%", "Approach play has stayed flat."],
};

export const lab = {
  kicker: "Coaching focus · week 1 of 2",
  title: "Turn short misses into GIR.",
  why: "Why: 63% of approach misses finish short.",
  target: "Target: +2 GIR per round.",
  sessions: [
    {
      id: "baseline",
      title: "Baseline ladder",
      detail: "Completed · 18 / 24 landing zones",
      done: true,
    },
    {
      id: "front-edge",
      title: "Front-edge challenge",
      detail: "35 min · 8 balls × 3 targets",
      done: false,
    },
    {
      id: "transfer",
      title: "Transfer round",
      detail: "One goal: choose enough club",
      done: false,
    },
  ],
};
