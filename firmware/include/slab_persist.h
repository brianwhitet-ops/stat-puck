#ifndef SLAB_PERSIST_H
#define SLAB_PERSIST_H

#include "slab_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int slab_persist_save(const slab_round_t *r);
int slab_persist_load(slab_round_t *r);
void slab_persist_set_host_path(const char *path);

#ifdef __cplusplus
}
#endif

#endif
