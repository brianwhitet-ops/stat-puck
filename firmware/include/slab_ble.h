#ifndef SLAB_BLE_H
#define SLAB_BLE_H

#include "slab_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Advertise is legal only in ROUND_COMPLETE_SYNC. Mid-round must stay silent. */
bool slab_ble_should_advertise(const slab_round_t *r);

/* protocol/README.md v2. Zero means failure; never return truncated JSON. */
int slab_ble_build_payload(const slab_round_t *r, char *out, int out_sz);
bool slab_ble_checksum(const slab_round_t *r, char out[15]);

#ifdef __cplusplus
}
#endif

#endif
