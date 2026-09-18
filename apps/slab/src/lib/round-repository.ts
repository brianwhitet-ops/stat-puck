import { upsertRound, type RoundPayload, type StoredRound } from "./round-contract";
import type { DurableRoundRepository } from "./round-import";

/** Browser/native-WebView adapter. No network or BLE dependency. */
export class IndexedDbRoundRepository implements DurableRoundRepository {
  private constructor(private db: IDBDatabase) {}
  static open(name = "slab-rounds-v2", factory: IDBFactory = indexedDB): Promise<IndexedDbRoundRepository> {
    return new Promise((resolve, reject) => {
      const req = factory.open(name, 1);
      req.onupgradeneeded = () => {
        req.result.createObjectStore("rounds", { keyPath: "payload.round_id" });
        req.result.createObjectStore("preserved", { autoIncrement: true });
      };
      req.onerror = () => reject(req.error);
      req.onblocked = () => reject(new Error("Close the other Slab window before upgrading storage"));
      req.onsuccess = () => {
        req.result.onversionchange = () => req.result.close();
        resolve(new IndexedDbRoundRepository(req.result));
      };
    });
  }
  private writeTransaction(storeName: string): IDBTransaction {
    const tx = this.db.transaction(storeName, "readwrite", { durability: "strict" });
    // Older engines may silently ignore the options dictionary. Require the
    // transaction to report strict durability before writing or sending an ACK.
    if (tx.durability !== "strict") {
      tx.abort();
      throw new Error("Storage does not support strict durability");
    }
    return tx;
  }
  commit(payload: RoundPayload, receivedAt: string): Promise<{ duplicate: boolean }> {
    const captured = structuredClone(payload);
    return new Promise((resolve, reject) => {
      const tx = this.writeTransaction("rounds");
      const store = tx.objectStore("rounds");
      let duplicate = false;
      let failure: unknown;
      tx.oncomplete = () => resolve({ duplicate });
      tx.onabort = () => reject(failure ?? tx.error ?? new Error("Import transaction aborted"));
      tx.onerror = () => { failure ??= tx.error; };
      const req = store.get(captured.round_id);
      req.onsuccess = () => {
        try {
          const result = upsertRound(req.result ? [req.result as StoredRound] : [], captured, receivedAt);
          duplicate = result.duplicate;
          store.put(result.store[0]);
        } catch (error) { failure = error; tx.abort(); }
      };
    });
  }
  preserve(raw: Uint8Array, reason: string): Promise<void> {
    return new Promise((resolve, reject) => {
      const tx = this.writeTransaction("preserved");
      tx.oncomplete = () => resolve();
      tx.onabort = () => reject(tx.error ?? new Error("Preservation failed"));
      tx.objectStore("preserved").add({ raw: raw.slice(), reason, received_at: new Date().toISOString() });
    });
  }
  close(): void { this.db.close(); }
}
