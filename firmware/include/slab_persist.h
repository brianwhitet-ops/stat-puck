#ifndef SLAB_PERSIST_H
#define SLAB_PERSIST_H
#include "slab_store.h"
#ifdef __cplusplus
extern "C" {
#endif
/* EMPTY is a new device. ERROR must never trigger a silent reset. */
enum { SLAB_PERSIST_OK = 0, SLAB_PERSIST_EMPTY = 1, SLAB_PERSIST_ERROR = -1 };
int slab_persist_save_store(const slab_store_t *store);
int slab_persist_load_store(slab_store_t *store);
void slab_persist_set_host_path(const char *path);
/* HAL: read exactly len bytes: 0 success, 1 absent, -1 bad size/I/O.
 * write must flush and close before returning 0. Never touch the other slot. */
int slab_persist_slot_read(unsigned slot, void *data, unsigned len);
int slab_persist_slot_write(unsigned slot, const void *data, unsigned len);
#ifdef __cplusplus
}
#endif
#endif
