#ifndef SLAB_BLE_H
#define SLAB_BLE_H

#include "slab_types.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Advertise is legal only in ROUND_COMPLETE_SYNC. Mid-round must stay silent. */
bool slab_ble_should_advertise(const slab_round_t *r);

/* protocol/README.md v2. Zero means failure; never return truncated JSON. */
int slab_ble_build_payload(const slab_round_t *r, char *out, int out_sz);
bool slab_ble_checksum(const slab_round_t *r, char out[15]);

/* Transport wrapper around the v2 round. Extra keys are not part of the round checksum.
 * Writing this text to the ACK characteristic is a valid retirement request. */
int slab_ble_build_meta(const slab_round_t *r, int byte_length, int page_length, int long_read,
                        char *out, int out_sz);
/* Five RoundAck fields. Ignores byte_length / page_length / long_read. */
bool slab_ble_parse_ack(const char *json, char *device, size_t device_cap, char *round_id,
                        size_t round_cap, uint32_t *sequence, char checksum[15]);

#ifdef __cplusplus
}
#endif

#endif
