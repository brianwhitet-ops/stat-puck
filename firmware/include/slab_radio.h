#ifndef SLAB_RADIO_H
#define SLAB_RADIO_H

#include "slab_store.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Bluefruit GATT for the sync screen. Host builds do not link this. */
void slab_radio_init(void);

/* Publish the oldest queued round and advertise only when `advertise` is true.
 * Passing false clears the readable body so a phone cannot pull mid-round. */
void slab_radio_apply(const slab_store_t *store, bool advertise);

/* 1 if no ACK write is waiting. 0 if `candidate` retired its oldest round.
 * Negative slab_store status otherwise. Does not persist. */
int slab_radio_consume_ack(slab_store_t *candidate);

void slab_radio_set_status(const char *status);
void slab_radio_sleep(void);

#ifdef __cplusplus
}
#endif

#endif
