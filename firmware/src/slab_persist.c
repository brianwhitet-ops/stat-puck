#include "slab_persist.h"
#include "slab_state.h"

#include <stdio.h>
#include <string.h>

#ifndef SLAB_HOST
#define SLAB_HOST 0
#endif

#if SLAB_HOST
static char g_host_path[256] = "firmware/testdata/nvram.bin";

void slab_persist_set_host_path(const char *path)
{
    if (!path) {
        return;
    }
    size_t n = 0;
    while (path[n] && n < sizeof(g_host_path) - 1) {
        g_host_path[n] = path[n];
        n++;
    }
    g_host_path[n] = 0;
}

int slab_persist_save(const slab_round_t *r)
{
    FILE *f = fopen(g_host_path, "wb");
    if (!f) {
        return -1;
    }
    size_t n = fwrite(r, 1, sizeof(*r), f);
    fclose(f);
    return n == sizeof(*r) ? 0 : -1;
}

int slab_persist_load(slab_round_t *r)
{
    FILE *f = fopen(g_host_path, "rb");
    if (!f) {
        return -1;
    }
    slab_round_t tmp;
    size_t n = fread(&tmp, 1, sizeof(tmp), f);
    fclose(f);
    if (n != sizeof(tmp) || tmp.magic != SLAB_PERSIST_MAGIC ||
        tmp.version != SLAB_PERSIST_VERSION) {
        return -1;
    }
    memcpy(r, &tmp, sizeof(tmp));
    slab_sync_ble_gate(r);
    return 0;
}
#else
/* Device: one flash page via weak hooks so the core links without HAL. */
void slab_persist_set_host_path(const char *path)
{
    (void)path;
}

__attribute__((weak)) int slab_persist_device_write(const void *data, unsigned len)
{
    (void)data;
    (void)len;
    return -1;
}

__attribute__((weak)) int slab_persist_device_read(void *data, unsigned len)
{
    (void)data;
    (void)len;
    return -1;
}

int slab_persist_save(const slab_round_t *r)
{
    return slab_persist_device_write(r, (unsigned)sizeof(*r));
}

int slab_persist_load(slab_round_t *r)
{
    slab_round_t tmp;
    if (slab_persist_device_read(&tmp, (unsigned)sizeof(tmp)) != 0) {
        return -1;
    }
    if (tmp.magic != SLAB_PERSIST_MAGIC || tmp.version != SLAB_PERSIST_VERSION) {
        return -1;
    }
    memcpy(r, &tmp, sizeof(tmp));
    slab_sync_ble_gate(r);
    return 0;
}
#endif
