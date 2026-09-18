/** Shared wire contract. See README.md. No radio or bootloader implementation. */
export const SCHEMA_VERSION = 2 as const;
export const MAX_PAYLOAD_BYTES = 4096;
export type DriveResult = "left" | "fairway" | "right" | null;
export type HolesPlayed = 9 | 18;
export interface HolePayload {
  hole: number;
  par: number | null;
  strokes: number | null;
  putts: number | null;
  drive_result: DriveResult;
}
export interface RoundPayload {
  schema_version: typeof SCHEMA_VERSION;
  round_id: string;
  device_id: string;
  round_sequence: number;
  holes_played: HolesPlayed;
  holes: HolePayload[];
  totals: { strokes: number | null; putts: number | null };
  checksum: string;
}
export interface RoundAck {
  schema_version: typeof SCHEMA_VERSION;
  round_id: string;
  device_id: string;
  round_sequence: number;
  checksum: string;
}
export type ValidateResult =
  | { ok: true; value: RoundPayload }
  | { ok: false; error: string; preserve: true };

export function holesPlayedValid(value: unknown): value is HolesPlayed {
  return value === 9 || value === 18;
}
export function driveResultValid(par: number | null, drive: unknown): drive is DriveResult {
  if (par === 3) return drive === null;
  return drive === null || drive === "left" || drive === "fairway" || drive === "right";
}
/** Null is unknown, never a missed green or a captured zero. */
export function deriveGir(h: Pick<HolePayload, "par" | "strokes" | "putts">): boolean | null {
  if (h.par === null || h.strokes === null || h.putts === null) return null;
  return h.strokes - h.putts <= h.par - 2;
}
/** Field order and ASCII identifier restrictions make this byte-identical to C. */
export function canonicalBody(p: Omit<RoundPayload, "checksum">): string {
  return JSON.stringify({
    schema_version: p.schema_version,
    round_id: p.round_id,
    device_id: p.device_id,
    round_sequence: p.round_sequence,
    holes_played: p.holes_played,
    holes: p.holes.map(h => ({
      hole: h.hole, par: h.par, strokes: h.strokes, putts: h.putts,
      drive_result: h.drive_result,
    })),
    totals: { strokes: p.totals.strokes, putts: p.totals.putts },
  });
}
/** CRC-32/ISO-HDLC. Corruption detection only, not authentication. */
export function crc32(bytes: Uint8Array): number {
  let crc = 0xffffffff;
  for (const byte of bytes) {
    crc ^= byte;
    for (let bit = 0; bit < 8; bit++) crc = (crc >>> 1) ^ (crc & 1 ? 0xedb88320 : 0);
  }
  return (crc ^ 0xffffffff) >>> 0;
}
export function computeChecksum(p: Omit<RoundPayload, "checksum">): string {
  return "crc32:" + crc32(new TextEncoder().encode(canonicalBody(p))).toString(16).padStart(8, "0");
}
export function encodeRound(p: RoundPayload): string {
  return canonicalBody(p).slice(0, -1) + ',"checksum":' + JSON.stringify(p.checksum) + "}";
}
function record(v: unknown): v is Record<string, unknown> {
  return v !== null && typeof v === "object" && !Array.isArray(v);
}
function exactKeys(v: Record<string, unknown>, keys: string[]): boolean {
  return Object.keys(v).length === keys.length && keys.every(k => Object.hasOwn(v, k));
}
function integer(v: unknown, lo: number, hi: number): v is number {
  return typeof v === "number" && Number.isInteger(v) && v >= lo && v <= hi;
}
function nullableInt(v: unknown, lo: number, hi: number): v is number | null {
  return v === null || integer(v, lo, hi);
}
function identifier(v: unknown, max: number): v is string {
  return typeof v === "string" && v.length <= max && /^[A-Za-z0-9:_-]+$/.test(v);
}
export function validateRoundPayload(input: unknown): ValidateResult {
  const fail = (error: string): ValidateResult => ({ ok: false, error, preserve: true });
  if (!record(input)) return fail("payload missing");
  if (input.schema_version !== SCHEMA_VERSION) return fail("unknown schema: preserve raw payload and require app update");
  if (!exactKeys(input, ["schema_version", "round_id", "device_id", "round_sequence", "holes_played", "holes", "totals", "checksum"]))
    return fail("unsupported or missing fields: preserve raw payload");
  if (!identifier(input.round_id, 64) || !identifier(input.device_id, 48)) return fail("invalid identity");
  if (!integer(input.round_sequence, 1, 0xffffffff)) return fail("invalid round_sequence");
  if (!holesPlayedValid(input.holes_played)) return fail("holes_played must be numeric 9 or 18");
  if (!Array.isArray(input.holes) || input.holes.length !== input.holes_played) return fail("all numbered holes required");
  for (let i = 0; i < input.holes.length; i++) {
    const h: unknown = input.holes[i];
    if (!record(h) || !exactKeys(h, ["hole", "par", "strokes", "putts", "drive_result"]) ||
        h.hole !== i + 1 || !nullableInt(h.par, 3, 5) || !nullableInt(h.strokes, 1, 15) ||
        !nullableInt(h.putts, 0, 10) || !driveResultValid(h.par, h.drive_result)) return fail("invalid hole");
    if (h.putts !== null && (h.strokes === null || h.putts > h.strokes)) return fail("putts require captured strokes and cannot exceed them");
  }
  if (!record(input.totals) || !exactKeys(input.totals, ["strokes", "putts"])) return fail("totals required");
  const holes = input.holes as HolePayload[];
  for (const field of ["strokes", "putts"] as const) {
    const total = holes.every(h => h[field] !== null) ? holes.reduce((sum, h) => sum + h[field]!, 0) : null;
    if (input.totals[field] !== total) return fail("totals disagree with captured fields");
  }
  const p = input as unknown as RoundPayload;
  if (typeof p.checksum !== "string" || !/^crc32:[0-9a-f]{8}$/.test(p.checksum) || p.checksum !== computeChecksum(p))
    return fail("checksum mismatch: preserve payload and retry");
  if (new TextEncoder().encode(encodeRound(p)).length > MAX_PAYLOAD_BYTES) return fail("payload too large");
  return { ok: true, value: p };
}
export function ackFor(p: RoundPayload): RoundAck {
  return { schema_version: SCHEMA_VERSION, round_id: p.round_id, device_id: p.device_id,
    round_sequence: p.round_sequence, checksum: p.checksum };
}

/** Ordered, bounded receive buffer. Matching duplicate chunks are safe to replay. */
export class RoundReceiver {
  private bytes: Uint8Array;
  private received = 0;
  readonly expected: RoundAck;
  constructor(expected: RoundAck, byteLength: number) {
    this.expected = structuredClone(expected);
    if (!Number.isInteger(byteLength) || byteLength < 1 || byteLength > MAX_PAYLOAD_BYTES) throw new Error("invalid transfer length");
    this.bytes = new Uint8Array(byteLength);
  }
  get offset(): number { return this.received; }
  append(offset: number, chunk: Uint8Array): void {
    if (!Number.isInteger(offset) || offset < 0 || offset > this.received || chunk.length === 0 || offset + chunk.length > this.bytes.length)
      throw new Error("invalid chunk: restart or retry from last contiguous offset");
    for (let i = 0; i < chunk.length; i++) {
      const at = offset + i;
      if (at < this.received && this.bytes[at] !== chunk[i]) throw new Error("conflicting replay");
    }
    this.bytes.set(chunk, offset);
    this.received = Math.max(this.received, offset + chunk.length);
  }
  raw(): Uint8Array { return this.bytes.slice(0, this.received); }
  finish(): RoundPayload {
    if (this.received !== this.bytes.length) throw new Error("incomplete transfer: no ACK");
    const raw: unknown = JSON.parse(new TextDecoder("utf-8", { fatal: true }).decode(this.bytes));
    const result = validateRoundPayload(raw);
    if (!result.ok) throw new Error(result.error);
    const p = result.value;
    if (p.round_id !== this.expected.round_id || p.device_id !== this.expected.device_id ||
        p.round_sequence !== this.expected.round_sequence || p.checksum !== this.expected.checksum ||
        this.expected.schema_version !== SCHEMA_VERSION) throw new Error("transfer identity changed: no ACK");
    return p;
  }
}
