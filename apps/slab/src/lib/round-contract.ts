/**
 * Slab puck → app round contract v0.1
 * Source of truth: design/instinct-app-handoff/puck-app-round-contract-v0.1.md
 *
 * Product boundary: the puck is the in-round source of truth. The phone
 * imports only after ROUND_COMPLETE_SYNC. No live-round phone dependency.
 *
 * v1 BLE import requires the app to be open. The puck keeps unacknowledged
 * rounds across disconnect and power-loss. ACK only after a durable write.
 *
 * Checkpoint D: these helpers are the contract scaffold. Visual parity (C)
 * does not depend on them being wired to BLE.
 */

export const SCHEMA_VERSION = 1 as const;

export type DriveResult = "left" | "fairway" | "right" | null;
export type HolesPlayed = 9 | 18;
export type FieldProvenance = "captured_on_slab" | "edited_in_app";

/** App-open import card — not a background-sync promise. */
export type ImportState =
  | "not_paired"
  | "pairing"
  | "connected"
  | "importing"
  | "imported"
  | "acknowledged"
  | "retry"
  | "app_update_required";

export interface HolePayload {
  hole: number;
  par: number;
  strokes: number;
  putts: number;
  /** Null on par 3. On par 4/5 only if the golfer supplied it. */
  drive_result: DriveResult;
}

export interface RoundPayload {
  schema_version: typeof SCHEMA_VERSION;
  round_id: string;
  device_id: string;
  started_at?: string;
  completed_at?: string;
  holes_played: HolesPlayed;
  holes: HolePayload[];
  totals: { strokes: number; putts: number };
  checksum: string;
}

export interface StoredRound {
  payload: RoundPayload;
  /** Oldest-first queue key. Never merge Saturday into Sunday. */
  queued_at: string;
  import_state: ImportState;
  /** Hole-level edits must not silently rewrite puck history. */
  field_provenance: Record<string, FieldProvenance>;
}

export type ValidateResult =
  | { ok: true; value: RoundPayload }
  | { ok: false; error: string; preserve: true };

export function deriveGir(hole: Pick<HolePayload, "par" | "strokes" | "putts">): boolean {
  return hole.strokes - hole.putts <= hole.par - 2;
}

export function driveResultValid(par: number, drive: DriveResult): boolean {
  if (par === 3) return drive === null;
  return drive === null || drive === "left" || drive === "fairway" || drive === "right";
}

export function holesPlayedValid(value: number): value is HolesPlayed {
  return value === 9 || value === 18;
}

/** Explicit 9 or 18 only. Never infer from inactivity or hole count entered. */
export function resolveHolesPlayed(startedAs: HolesPlayed, endedNowAsNine: boolean): HolesPlayed {
  if (startedAs === 18 && endedNowAsNine) return 9;
  return startedAs;
}

export function computeChecksum(payload: Omit<RoundPayload, "checksum">): string {
  const body = JSON.stringify({
    schema_version: payload.schema_version,
    round_id: payload.round_id,
    device_id: payload.device_id,
    holes_played: payload.holes_played,
    holes: payload.holes,
    totals: payload.totals,
  });
  let hash = 0;
  for (let i = 0; i < body.length; i += 1) {
    hash = (hash * 31 + body.charCodeAt(i)) >>> 0;
  }
  return hash.toString(16).padStart(8, "0");
}

function isHole(value: unknown): value is HolePayload {
  if (!value || typeof value !== "object") return false;
  const hole = value as Partial<HolePayload>;
  return (
    typeof hole.hole === "number" &&
    typeof hole.par === "number" &&
    typeof hole.strokes === "number" &&
    typeof hole.putts === "number" &&
    driveResultValid(hole.par, (hole.drive_result ?? null) as DriveResult)
  );
}

export function validateRoundPayload(input: unknown): ValidateResult {
  if (!input || typeof input !== "object") {
    return { ok: false, error: "payload missing", preserve: true };
  }
  const raw = input as Partial<RoundPayload> & { schema_version?: number };
  if (raw.schema_version !== SCHEMA_VERSION) {
    return { ok: false, error: "unknown schema — keep on puck, require app update", preserve: true };
  }
  if (typeof raw.round_id !== "string" || raw.round_id.length === 0) {
    return { ok: false, error: "round_id required", preserve: true };
  }
  if (typeof raw.device_id !== "string" || raw.device_id.length === 0) {
    return { ok: false, error: "device_id required", preserve: true };
  }
  if (!holesPlayedValid(Number(raw.holes_played))) {
    return { ok: false, error: "holes_played must be 9 or 18", preserve: true };
  }
  if (!Array.isArray(raw.holes) || raw.holes.length === 0 || !raw.holes.every(isHole)) {
    return { ok: false, error: "holes invalid — do not partially create a round", preserve: true };
  }
  if (!raw.totals || typeof raw.totals.strokes !== "number" || typeof raw.totals.putts !== "number") {
    return { ok: false, error: "totals required", preserve: true };
  }
  if (typeof raw.checksum !== "string") {
    return { ok: false, error: "checksum required", preserve: true };
  }

  const candidate = raw as RoundPayload;
  const expected = computeChecksum(candidate);
  if (candidate.checksum !== expected) {
    return { ok: false, error: "checksum mismatch — preserve payload, show Retry", preserve: true };
  }

  return { ok: true, value: candidate };
}

/**
 * Idempotent write by round_id. Duplicates update sync metadata only.
 * ACK is the caller's job after this returns ok.
 */
export function upsertRound(
  store: StoredRound[],
  payload: RoundPayload,
  queuedAt: string,
): { store: StoredRound[]; duplicate: boolean } {
  const existing = store.findIndex((row) => row.payload.round_id === payload.round_id);
  if (existing >= 0) {
    const next = store.slice();
    next[existing] = {
      ...next[existing],
      payload,
      import_state: "imported",
    };
    return { store: next, duplicate: true };
  }
  return {
    store: [
      ...store,
      {
        payload,
        queued_at: queuedAt,
        import_state: "imported",
        field_provenance: {},
      },
    ],
    duplicate: false,
  };
}

/** Import the queue oldest-first. Never merge similar course/date rows. */
export function queuedOldestFirst(store: StoredRound[]): StoredRound[] {
  return store.slice().sort((a, b) => a.queued_at.localeCompare(b.queued_at));
}

export function markEditedInApp(
  round: StoredRound,
  field: string,
): StoredRound {
  return {
    ...round,
    field_provenance: {
      ...round.field_provenance,
      [field]: "edited_in_app",
    },
  };
}
