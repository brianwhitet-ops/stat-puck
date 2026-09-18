import { describe, expect, it, vi } from "vitest";
import { IDBFactory, IDBDatabase as FakeIDBDatabase } from "fake-indexeddb";
import { ackFor, encodeRound, RoundReceiver, computeChecksum, type RoundPayload } from "./round-contract";
import { importAndAcknowledge, type DurableRoundRepository } from "./round-import";
import { IndexedDbRoundRepository } from "./round-repository";
import fixture from "../../../../protocol/fixtures/eighteen.json";
function sample(): RoundPayload { return structuredClone(fixture) as RoundPayload; }
function receiver(p = sample(), raw?: string): RoundReceiver {
  const bytes = new TextEncoder().encode(raw ?? encodeRound(p));
  const result = new RoundReceiver(ackFor(p), bytes.length);
  result.append(0, bytes);
  return result;
}
const date = "2026-09-18T10:00:00Z";
describe("durable round import", () => {
  it("waits for commit completion before sending ACK", async () => {
    const events: string[] = [];
    let commit: (() => void) | undefined;
    const repo: DurableRoundRepository = {
      commit: async () => { events.push("write-start"); await new Promise<void>(r => { commit = r; }); events.push("write-complete"); return { duplicate: false }; },
      preserve: async () => { events.push("preserve"); },
    };
    const pending = importAndAcknowledge(receiver(), repo, async () => { events.push("ack"); }, date);
    expect(events).toEqual(["write-start"]);
    commit!(); expect((await pending).state).toBe("acknowledged");
    expect(events).toEqual(["write-start", "write-complete", "ack"]);
  });
  it("does not ACK a failed durable write", async () => {
    let acks = 0;
    const repo: DurableRoundRepository = { commit: async () => { throw new Error("disk full"); }, preserve: async () => {} };
    expect((await importAndAcknowledge(receiver(), repo, async () => { acks++; }, date)).state).toBe("retry");
    expect(acks).toBe(0);
  });
  it("preserves unknown schemas and corrupt input without ACK", async () => {
    const preserved: string[] = [];
    let commits = 0, acks = 0;
    const repo: DurableRoundRepository = { commit: async () => { commits++; return { duplicate: false }; },
      preserve: async raw => { preserved.push(new TextDecoder().decode(raw)); } };
    const unknown = JSON.stringify({ ...sample(), schema_version: 99 });
    expect((await importAndAcknowledge(receiver(sample(), unknown), repo, async () => { acks++; }, date)).state).toBe("app_update_required");
    const corrupt = encodeRound(sample()).replace('"strokes":4', '"strokes":15');
    expect((await importAndAcknowledge(receiver(sample(), corrupt), repo, async () => { acks++; }, date)).state).toBe("retry");
    expect(preserved).toEqual([unknown, corrupt]); expect(commits).toBe(0); expect(acks).toBe(0);
  });
  it("recovers a lost ACK through an idempotent transaction after reopen", async () => {
    const factory = new IDBFactory();
    let repo = await IndexedDbRoundRepository.open("ack-retry", factory);
    expect((await importAndAcknowledge(receiver(), repo, async () => { throw new Error("disconnected"); }, date)).state).toBe("retry");
    repo.close(); repo = await IndexedDbRoundRepository.open("ack-retry", factory);
    const acks: unknown[] = [];
    expect(await importAndAcknowledge(receiver(), repo, async ack => { acks.push(ack); }, date)).toEqual({ state: "acknowledged", duplicate: true });
    expect(acks).toEqual([ackFor(sample())]); repo.close();
  });
  it("serializes concurrent duplicates and rejects a conflicting payload", async () => {
    const repo = await IndexedDbRoundRepository.open("atomic", new IDBFactory());
    const results = await Promise.all([repo.commit(sample(), date), repo.commit(sample(), date)]);
    expect(results.map(r => r.duplicate).sort()).toEqual([false, true]);
    const conflict = sample(); conflict.holes[0].strokes!++; conflict.totals.strokes!++;
    conflict.checksum = computeChecksum(conflict);
    await expect(repo.commit(conflict, date)).rejects.toThrow("round_id conflict");
    expect((await repo.commit(sample(), date)).duplicate).toBe(true); repo.close();
  });
  it("fails closed after closing the database", async () => {
    const repo = await IndexedDbRoundRepository.open("closed", new IDBFactory()); repo.close();
    let acks = 0;
    expect((await importAndAcknowledge(receiver(), repo, async () => { acks++; }, date)).state).toBe("retry");
    expect(acks).toBe(0);
  });
  it("does not ACK when storage silently ignores strict durability", async () => {
    const repo = await IndexedDbRoundRepository.open("ignored-durability", new IDBFactory());
    const original = FakeIDBDatabase.prototype.transaction;
    const mock = vi.spyOn(FakeIDBDatabase.prototype, "transaction").mockImplementation(function (this: InstanceType<typeof FakeIDBDatabase>, names, mode) {
      return original.call(this, names, mode);
    });
    let acks = 0;
    try {
      const result = await importAndAcknowledge(receiver(), repo, async () => { acks++; }, date);
      expect(result).toEqual({ state: "retry", error: "Storage does not support strict durability" });
      expect(acks).toBe(0);
      await expect(repo.preserve(new Uint8Array([1]), "test")).rejects.toThrow("strict durability");
    } finally { mock.mockRestore(); repo.close(); }
  });
});
