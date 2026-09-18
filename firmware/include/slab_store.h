#ifndef SLAB_STORE_H
#define SLAB_STORE_H
#include "slab_types.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    uint32_t next_sequence;
    uint8_t active_present;
    uint8_t count;
    slab_round_t active;
    slab_round_t queued[SLAB_QUEUE_CAPACITY];
} slab_store_t;
enum { SLAB_STORE_OK = 0, SLAB_STORE_FULL = -1, SLAB_STORE_INVALID = -2,
       SLAB_STORE_NOT_FOUND = -3, SLAB_STORE_CONFLICT = -4 };
void slab_store_init(slab_store_t *store);
bool slab_store_valid(const slab_store_t *store);
int slab_store_begin(slab_store_t *store, const char *device, uint8_t holes, const uint8_t *pars);
int slab_store_finish(slab_store_t *store);
/* Mutates a candidate snapshot only. Publish it only after persistence succeeds. */
int slab_store_ack(slab_store_t *store, const char *device, const char *round_id,
                   uint32_t sequence, const char *checksum);
/* Non-destructive transfer. Restarting/re-reading never removes the round. */
int slab_store_read_chunk(const slab_store_t *store, size_t offset, void *out, size_t cap, size_t *total);
#ifdef __cplusplus
}
#endif
#endif
