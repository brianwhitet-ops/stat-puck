#include "slab_store.h"
#include "slab_state.h"
#include "slab_ble.h"
#include <string.h>
#include <stdint.h>

void slab_store_init(slab_store_t *s)
{
    memset(s, 0, sizeof(*s));
    s->next_sequence = 1;
}
bool slab_store_valid(const slab_store_t *s)
{
    if (!s || !s->next_sequence || s->count > SLAB_QUEUE_CAPACITY || s->active_present > 1) return false;
    if (s->active_present && (!slab_round_valid(&s->active) || s->active.round_sequence >= s->next_sequence)) return false;
    for (int i = 0; i < s->count; i++) {
        const slab_round_t *r = &s->queued[i];
        if (!slab_round_complete(r) || r->round_sequence >= s->next_sequence) return false;
        if (i && (r->round_sequence <= s->queued[i - 1].round_sequence || strcmp(r->device_id, s->queued[0].device_id))) return false;
        if (s->active_present && (r->round_sequence >= s->active.round_sequence || strcmp(r->device_id, s->active.device_id))) return false;
    }
    return true;
}
int slab_store_begin(slab_store_t *s, const char *device, uint8_t holes, const uint8_t *pars)
{
    if (!device || !slab_store_valid(s) || s->active_present || s->next_sequence == UINT32_MAX) return SLAB_STORE_INVALID;
    if (s->count == SLAB_QUEUE_CAPACITY) return SLAB_STORE_FULL;
    if (s->count && strcmp(device, s->queued[0].device_id)) return SLAB_STORE_INVALID;
    slab_round_t r;
    if (slab_round_start(&r, device, s->next_sequence, holes, pars)) return SLAB_STORE_INVALID;
    s->active = r;
    s->active_present = 1;
    s->next_sequence++;
    return 0;
}
int slab_store_finish(slab_store_t *s)
{
    if (!slab_store_valid(s) || !s->active_present || !slab_round_complete(&s->active)) return SLAB_STORE_INVALID;
    if (s->count == SLAB_QUEUE_CAPACITY) return SLAB_STORE_FULL;
    s->queued[s->count++] = s->active;
    s->active_present = 0;
    memset(&s->active, 0, sizeof(s->active));
    return 0;
}
int slab_store_ack(slab_store_t *s, const char *device, const char *id, uint32_t sequence, const char *checksum)
{
    if (!slab_store_valid(s) || !device || !id || !checksum || s->active_present) return SLAB_STORE_INVALID;
    if (!s->count) return SLAB_STORE_NOT_FOUND;
    const slab_round_t *r = &s->queued[0];
    if (strcmp(device, r->device_id) || strcmp(id, r->round_id) || sequence != r->round_sequence) return SLAB_STORE_CONFLICT;
    char expected[15];
    if (!slab_ble_checksum(r, expected) || strcmp(checksum, expected)) return SLAB_STORE_CONFLICT;
    memmove(s->queued, s->queued + 1, (s->count - 1) * sizeof(s->queued[0]));
    memset(&s->queued[--s->count], 0, sizeof(s->queued[0]));
    return 0;
}
int slab_store_read_chunk(const slab_store_t *s, size_t offset, void *out, size_t cap, size_t *total)
{
    if (!slab_store_valid(s) || s->active_present || !s->count || !out || !cap || !total) return -1;
    static char payload[SLAB_PAYLOAD_SIZE]; /* serialized task context, not ISR */
    int n = slab_ble_build_payload(&s->queued[0], payload, sizeof(payload));
    if (n <= 0 || offset > (size_t)n) return -1;
    *total = (size_t)n;
    size_t count = (size_t)n - offset;
    if (count > cap) count = cap;
    memcpy(out, payload + offset, count);
    return (int)count;
}
