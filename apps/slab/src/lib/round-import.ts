import { ackFor, RoundReceiver, type RoundAck, type RoundPayload } from "./round-contract";

export interface DurableRoundRepository {
  /** Resolve only after a durable, atomic, idempotent transaction commits. */
  commit(payload: RoundPayload, receivedAt: string): Promise<{ duplicate: boolean }>;
  preserve(raw: Uint8Array, reason: string): Promise<void>;
}
export type ImportResult =
  | { state: "acknowledged"; duplicate: boolean }
  | { state: "retry" | "app_update_required"; error: string };

/** sendAck resolves only on the device's persisted ACK result, not an ATT write. */
export async function importAndAcknowledge(
  receiver: RoundReceiver,
  repository: DurableRoundRepository,
  sendAck: (ack: RoundAck) => Promise<void>,
  receivedAt: string,
): Promise<ImportResult> {
  let payload: RoundPayload;
  try { payload = receiver.finish(); }
  catch (error) {
    const reason = error instanceof Error ? error.message : String(error);
    try { await repository.preserve(receiver.raw(), reason); }
    catch { return { state: "retry", error: "Could not preserve received data; round remains on puck" }; }
    return { state: reason.includes("unknown schema") ? "app_update_required" : "retry", error: reason };
  }
  try {
    const { duplicate } = await repository.commit(payload, receivedAt);
    await sendAck(ackFor(payload));
    return { state: "acknowledged", duplicate };
  } catch (error) {
    return { state: "retry", error: error instanceof Error ? error.message : String(error) };
  }
}
