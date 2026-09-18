#if SLAB_HOST
#define _POSIX_C_SOURCE 200809L
#endif
#include "slab_persist.h"
#include "slab_crc32.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#if SLAB_HOST
#include <unistd.h>
#include <errno.h>
#endif

#define JOURNAL_MAGIC 0x534c4a32u
/* Persisted ABI is versioned and sizeof-checked. Future firmware needs an
 * explicit migration; it may not reinterpret the v1 prefilled round structs. */
typedef struct {
    uint32_t magic, version, generation, payload_size, crc;
    slab_store_t store;
} snapshot_t;
/* Serialized main-task API; fixed workspace avoids overflowing the XIAO loop stack. */
static snapshot_t g_best, g_scratch;
static uint32_t checksum(snapshot_t *snapshot)
{
    uint32_t saved = snapshot->crc;
    snapshot->crc = 0;
    uint32_t result = slab_crc32(snapshot, sizeof(*snapshot));
    snapshot->crc = saved;
    return result;
}
static bool valid(snapshot_t *s)
{
    return s->magic == JOURNAL_MAGIC && s->version == SLAB_PERSIST_VERSION &&
           s->generation && s->payload_size == sizeof(s->store) &&
           s->crc == checksum(s) && slab_store_valid(&s->store);
}
/* A damaged inactive slot is recoverable. Two bad slots are never "empty". */
static int latest(snapshot_t *best, unsigned *slot)
{
    snapshot_t *candidate = &g_scratch;
    bool found = false, all_absent = true;
    for (unsigned i = 0; i < 2; i++) {
        int status = slab_persist_slot_read(i, candidate, sizeof(*candidate));
        if (status != SLAB_PERSIST_EMPTY) all_absent = false;
        if (status == 0 && valid(candidate)) {
            if (!found || candidate->generation > best->generation) {
                *best = *candidate;
                *slot = i;
                found = true;
            } else if (candidate->generation == best->generation && memcmp(best, candidate, sizeof(*candidate))) {
                return SLAB_PERSIST_ERROR;
            }
        }
    }
    if (found) return 0;
    return all_absent ? SLAB_PERSIST_EMPTY : SLAB_PERSIST_ERROR;
}
int slab_persist_load_store(slab_store_t *store)
{
    if (!store) return SLAB_PERSIST_ERROR;
    snapshot_t *best = &g_best;
    unsigned slot = 0;
    int status = latest(best, &slot);
    if (!status) *store = best->store;
    return status;
}
int slab_persist_save_store(const slab_store_t *store)
{
    if (!slab_store_valid(store)) return SLAB_PERSIST_ERROR;
    snapshot_t *snap = &g_best;
    unsigned old_slot = 1;
    int status = latest(snap, &old_slot);
    if (status == SLAB_PERSIST_ERROR) return status;
    uint32_t generation = status == SLAB_PERSIST_EMPTY ? 0 : snap->generation;
    if (generation == UINT32_MAX) return SLAB_PERSIST_ERROR;
    memset(snap, 0, sizeof(*snap));
    snap->magic = JOURNAL_MAGIC;
    snap->version = SLAB_PERSIST_VERSION;
    snap->generation = generation + 1;
    snap->payload_size = sizeof(snap->store);
    snap->store = *store;
    snap->crc = checksum(snap);
    unsigned target = 1 - old_slot;
    if (slab_persist_slot_write(target, snap, sizeof(*snap))) return SLAB_PERSIST_ERROR;
    snapshot_t *verify = &g_scratch;
    if (slab_persist_slot_read(target, verify, sizeof(*verify)) || !valid(verify) ||
        memcmp(verify, snap, sizeof(*snap))) return SLAB_PERSIST_ERROR;
    return 0;
}

#if SLAB_HOST
static char g_host_path[240] = "/tmp/slab-store";
void slab_persist_set_host_path(const char *path)
{
    if (path) snprintf(g_host_path, sizeof(g_host_path), "%s", path);
}
__attribute__((weak)) int slab_persist_slot_read(unsigned slot, void *data, unsigned len)
{
    char path[256];
    snprintf(path, sizeof(path), "%s.%u", g_host_path, slot);
    FILE *f = fopen(path, "rb");
    if (!f) return errno == ENOENT ? SLAB_PERSIST_EMPTY : SLAB_PERSIST_ERROR;
    size_t got = fread(data, 1, len, f);
    int extra = fgetc(f);
    int failed = ferror(f);
    fclose(f);
    return got == len && extra == EOF && !failed ? 0 : -1;
}
__attribute__((weak)) int slab_persist_slot_write(unsigned slot, const void *data, unsigned len)
{
    char path[256];
    snprintf(path, sizeof(path), "%s.%u", g_host_path, slot);
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    bool ok = fwrite(data, 1, len, f) == len;
    if (fflush(f)) ok = false;
    if (fsync(fileno(f))) ok = false;
    if (fclose(f)) ok = false;
    return ok ? 0 : -1;
}
#else
void slab_persist_set_host_path(const char *path) { (void)path; }
__attribute__((weak)) int slab_persist_slot_read(unsigned slot, void *data, unsigned len)
{ (void)slot; (void)data; (void)len; return -1; }
__attribute__((weak)) int slab_persist_slot_write(unsigned slot, const void *data, unsigned len)
{ (void)slot; (void)data; (void)len; return -1; }
#endif
