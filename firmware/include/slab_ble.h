#ifndef SLAB_BLE_H
#define SLAB_BLE_H

#include "slab_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Advertise is legal only in ROUND_COMPLETE_SYNC. Mid-round must stay silent. */
bool slab_ble_should_advertise(const slab_round_t *r);

/* Fill a SYNC_PAYLOAD.md-shaped buffer. Returns bytes written (NUL-terminated). */
int slab_ble_build_payload(const slab_round_t *r, char *out, int out_sz);

#ifdef __cplusplus
}
#endif

#endif
