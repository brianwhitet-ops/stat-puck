import { describe, expect, it } from "vitest";
import {
  computeChecksum,
  deriveGir,
  driveResultValid,
  markEditedInApp,
  queuedOldestFirst,
  resolveHolesPlayed,
  upsertRound,
  validateRoundPayload,
  type RoundPayload,
  type StoredRound,
} from "./round-contract";

function samplePayload(overrides: Partial<RoundPayload> = {}): RoundPayload {
  const base: Omit<RoundPayload, "checksum"> = {
    schema_version: 1,
    round_id: "round-saturday",
    device_id: "slab-001",
    holes_played: 18,
    holes: [
      { hole: 1, par: 4, strokes: 5, putts: 2, drive_result: "left" },
      { hole: 3, par: 3, strokes: 3, putts: 2, drive_result: null },
    ],
    totals: { strokes: 82, putts: 31 },
    ...overrides,
  };
  const withHoles = { ...base, ...overrides };
  return {
    ...withHoles,
    checksum: overrides.checksum ?? computeChecksum(withHoles),
  };
}

describe("round contract v0.1 scaffold", () => {
  it("derives GIR from strokes and putts — not a puck input", () => {
    expect(deriveGir({ par: 4, strokes: 4, putts: 2 })).toBe(true);
    expect(deriveGir({ par: 4, strokes: 5, putts: 2 })).toBe(false);
  });

  it("forces drive_result null on par 3", () => {
    expect(driveResultValid(3, null)).toBe(true);
    expect(driveResultValid(3, "fairway")).toBe(false);
    expect(driveResultValid(4, "left")).toBe(true);
  });

  it("treats 9 and 18 as explicit, including End now at the turn", () => {
    expect(resolveHolesPlayed(18, true)).toBe(9);
    expect(resolveHolesPlayed(18, false)).toBe(18);
    expect(resolveHolesPlayed(9, false)).toBe(9);
  });

  it("rejects unknown schema and checksum failure without creating a round", () => {
    const unknown = validateRoundPayload({ ...samplePayload(), schema_version: 99 });
    expect(unknown.ok).toBe(false);
    if (!unknown.ok) expect(unknown.preserve).toBe(true);

    const badSum = validateRoundPayload({ ...samplePayload(), checksum: "deadbeef" });
    expect(badSum.ok).toBe(false);
  });

  it("upserts idempotently by round_id and will not merge a later round", () => {
    const saturday = samplePayload();
    const sunday = samplePayload({ round_id: "round-sunday" });
    const first = upsertRound([], saturday, "2026-09-12T18:00:00Z");
    const dup = upsertRound(first.store, saturday, "2026-09-13T18:00:00Z");
    const both = upsertRound(dup.store, sunday, "2026-09-13T18:00:00Z");

    expect(first.duplicate).toBe(false);
    expect(dup.duplicate).toBe(true);
    expect(both.store).toHaveLength(2);
    expect(both.store.map((row) => row.payload.round_id)).toEqual([
      "round-saturday",
      "round-sunday",
    ]);
  });

  it("imports queued rounds oldest-first", () => {
    const rows: StoredRound[] = [
      {
        payload: samplePayload({ round_id: "newer" }),
        queued_at: "2026-09-14T12:00:00Z",
        import_state: "imported",
        field_provenance: {},
      },
      {
        payload: samplePayload({ round_id: "older" }),
        queued_at: "2026-09-12T12:00:00Z",
        import_state: "imported",
        field_provenance: {},
      },
    ];
    expect(queuedOldestFirst(rows).map((row) => row.payload.round_id)).toEqual([
      "older",
      "newer",
    ]);
  });

  it("records app-edit provenance without rewriting captured history", () => {
    const stored: StoredRound = {
      payload: samplePayload(),
      queued_at: "2026-09-14T12:00:00Z",
      import_state: "acknowledged",
      field_provenance: { "holes.1.strokes": "captured_on_slab" },
    };
    const edited = markEditedInApp(stored, "holes.1.strokes");
    expect(edited.field_provenance["holes.1.strokes"]).toBe("edited_in_app");
    expect(stored.field_provenance["holes.1.strokes"]).toBe("captured_on_slab");
  });
});
