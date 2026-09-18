/** Device protocol is shared with the firmware workstream. Radio wiring is separate. */
export * from "../../../../protocol/round-v2";
import { canonicalBody, validateRoundPayload, type HolesPlayed, type RoundPayload } from "../../../../protocol/round-v2";
export type FieldProvenance = "captured_on_slab" | "edited_in_app";
export type ImportState = "not_paired" | "pairing" | "connected" | "importing" |
  "imported" | "acknowledged" | "retry" | "app_update_required";
export interface StoredRound {
  /** Immutable captured source. App edits belong in a separate overlay. */
  payload: RoundPayload;
  queued_at: string;
  import_state: ImportState;
  field_provenance: Record<string, FieldProvenance>;
  edits?: Record<string, number | string | null>;
}
export function resolveHolesPlayed(startedAs: HolesPlayed, endedNowAsNine: boolean): HolesPlayed {
  return startedAs === 18 && endedNowAsNine ? 9 : startedAs;
}
export function upsertRound(store: StoredRound[], payload: RoundPayload, queuedAt: string): { store: StoredRound[]; duplicate: boolean } {
  const checked = validateRoundPayload(payload);
  if (!checked.ok) throw new Error(checked.error);
  const existing = store.findIndex(row => row.payload.round_id === payload.round_id);
  if (existing >= 0) {
    const original = store[existing];
    if (canonicalBody(original.payload) !== canonicalBody(payload) || original.payload.checksum !== payload.checksum)
      throw new Error("round_id conflict: preserve both payloads, do not ACK");
    const next = store.slice();
    next[existing] = { ...original, import_state: "imported" };
    return { store: next, duplicate: true };
  }
  return { store: [...store, { payload: structuredClone(payload), queued_at: queuedAt,
    import_state: "imported", field_provenance: {} }], duplicate: false };
}
export function queuedOldestFirst(store: StoredRound[]): StoredRound[] {
  return store.slice().sort((a, b) => a.queued_at.localeCompare(b.queued_at));
}
export function markEditedInApp(round: StoredRound, field: string): StoredRound {
  return { ...round, field_provenance: { ...round.field_provenance, [field]: "edited_in_app" } };
}
