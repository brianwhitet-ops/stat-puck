#include "slab_persist.h"
#include "slab_state.h"
#include "slab_ble.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define SLOT_CAP 16384
static unsigned char slots[2][SLOT_CAP];
static unsigned sizes[2];
static bool present[2];
static int cut_after = -1;
static bool fail_after_full_write;
int slab_persist_slot_read(unsigned slot, void *data, unsigned len)
{
    assert(slot < 2 && len < SLOT_CAP);
    if (!present[slot]) return SLAB_PERSIST_EMPTY;
    if (sizes[slot] != len) return -1;
    memcpy(data, slots[slot], len);
    return 0;
}
int slab_persist_slot_write(unsigned slot, const void *data, unsigned len)
{
    assert(slot < 2 && len < SLOT_CAP);
    present[slot] = true;
    unsigned n = cut_after < 0 || (unsigned)cut_after >= len ? len : (unsigned)cut_after;
    sizes[slot] = n;
    memcpy(slots[slot], data, n);
    return n == len && !fail_after_full_write ? 0 : -1;
}
static void reset(void)
{
    memset(slots, 0, sizeof(slots)); memset(sizes, 0, sizeof(sizes));
    memset(present, 0, sizeof(present)); cut_after = -1; fail_after_full_write = false;
}
int main(void)
{
    slab_store_t original, updated, restored;
    reset(); slab_store_init(&original);
    assert(slab_persist_load_store(&restored) == SLAB_PERSIST_EMPTY);
    assert(!slab_store_begin(&original, "journal", 9, NULL));
    slab_apply_event(&original.active, SLAB_EVT_PLUS);
    assert(!slab_persist_save_store(&original));
    unsigned snapshot_size = sizes[0];
    updated = original;
    slab_apply_event(&updated.active, SLAB_EVT_PLUS);
    /* Every byte position of an interrupted inactive-slot write. */
    for (unsigned cut = 0; cut < snapshot_size; cut++) {
        cut_after = (int)cut;
        assert(slab_persist_save_store(&updated) == SLAB_PERSIST_ERROR);
        assert(!slab_persist_load_store(&restored));
        assert(!memcmp(&original, &restored, sizeof(original)));
    }
    cut_after = -1;
    assert(!slab_persist_save_store(&updated));
    assert(!slab_persist_load_store(&restored));
    assert(restored.active.holes[0].strokes == 2);
    // A corrupt newest record falls back to the last committed record.
    slots[1][snapshot_size - 1] ^= 1;
    assert(!slab_persist_load_store(&restored));
    assert(restored.active.holes[0].strokes == 1);
    slots[0][0] ^= 1;
    restored = original;
    assert(slab_persist_load_store(&restored) == SLAB_PERSIST_ERROR);
    assert(!memcmp(&restored, &original, sizeof(original)));
    assert(slab_persist_save_store(&updated) == SLAB_PERSIST_ERROR);
    // Failure reported after a complete write may recover the complete new state.
    reset(); assert(!slab_persist_save_store(&original));
    fail_after_full_write = true;
    assert(slab_persist_save_store(&updated) == SLAB_PERSIST_ERROR);
    assert(!slab_persist_load_store(&restored));
    assert(restored.active.holes[0].strokes == 2);
    // ACK retirement is also journaled; a torn commit leaves the queued round.
    reset(); slab_store_init(&original);
    original.next_sequence = 2;
    original.count = 1;
    slab_round_init_fixture_complete(&original.queued[0]);
    assert(!slab_persist_save_store(&original));
    char sum[15]; assert(slab_ble_checksum(&original.queued[0], sum));
    updated = original;
    assert(!slab_store_ack(&updated, "bench", original.queued[0].round_id, 1, sum));
    cut_after = 10;
    assert(slab_persist_save_store(&updated) == SLAB_PERSIST_ERROR);
    assert(!slab_persist_load_store(&restored) && restored.count == 1);
    cut_after = -1; assert(!slab_persist_save_store(&updated));
    assert(!slab_persist_load_store(&restored) && restored.count == 0);
    // Corrupt schema version is not silently treated as an empty device.
    reset(); assert(!slab_persist_save_store(&original)); slots[0][4] = 99;
    assert(slab_persist_load_store(&restored) == SLAB_PERSIST_ERROR);
    printf("journal: all %u interrupted-write offsets, corrupt slots and ACK retirement passed\n", snapshot_size);
    return 0;
}
