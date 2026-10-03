#include "slab_ble.h"
#include "slab_state.h"
#include "slab_crc32.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

bool slab_ble_should_advertise(const slab_round_t *r)
{
    return r && r->ui == SLAB_UI_ROUND_COMPLETE_SYNC && r->ble_advertise && slab_round_complete(r);
}
typedef struct { char *data; size_t cap, len; bool failed; } writer_t;
static void append(writer_t *w, const char *fmt, ...)
{
    if (w->failed) return;
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(w->data + w->len, w->cap - w->len, fmt, args);
    va_end(args);
    if (n < 0 || (size_t)n >= w->cap - w->len) { w->failed = true; return; }
    w->len += (size_t)n;
}
static void value(writer_t *w, unsigned n, bool known)
{
    if (known) append(w, "%u", n);
    else append(w, "null");
}
static const char *drive(const slab_hole_t *h)
{
    if (!(h->captured & SLAB_CAPTURE_DRIVE) || h->par == 3) return "null";
    if (h->fairway == SLAB_FWY_L) return "\"left\"";
    if (h->fairway == SLAB_FWY_H) return "\"fairway\"";
    if (h->fairway == SLAB_FWY_R) return "\"right\"";
    return "null";
}
int slab_ble_build_payload(const slab_round_t *r, char *out, int out_sz)
{
    if (!out || out_sz < 1) return 0;
    out[0] = 0;
    if (!slab_round_valid(r)) return 0;
    writer_t w = {out, (size_t)out_sz, 0, false};
    append(&w, "{\"schema_version\":2,\"round_id\":\"%s\",\"device_id\":\"%s\",\"round_sequence\":%lu,\"holes_played\":%u,\"holes\":[",
           r->round_id, r->device_id, (unsigned long)r->round_sequence, (unsigned)r->holes_played);
    unsigned strokes = 0, putts = 0;
    bool all_strokes = true, all_putts = true;
    for (int i = 0; i < r->holes_played; i++) {
        const slab_hole_t *h = &r->holes[i];
        append(&w, "%s{\"hole\":%u,\"par\":", i ? "," : "", (unsigned)h->hole);
        value(&w, h->par, h->par != 0);
        append(&w, ",\"strokes\":");
        value(&w, h->strokes, (h->captured & SLAB_CAPTURE_STROKES) != 0);
        append(&w, ",\"putts\":");
        value(&w, h->putts, (h->captured & SLAB_CAPTURE_PUTTS) != 0);
        append(&w, ",\"drive_result\":%s}", drive(h));
        all_strokes &= (h->captured & SLAB_CAPTURE_STROKES) != 0;
        all_putts &= (h->captured & SLAB_CAPTURE_PUTTS) != 0;
        strokes += h->strokes;
        putts += h->putts;
    }
    append(&w, "],\"totals\":{\"strokes\":");
    value(&w, strokes, all_strokes);
    append(&w, ",\"putts\":");
    value(&w, putts, all_putts);
    append(&w, "}}");
    if (w.failed) { out[0] = 0; return 0; }
    uint32_t crc = slab_crc32(out, w.len);
    w.len--;
    append(&w, ",\"checksum\":\"crc32:%08lx\"}", (unsigned long)crc);
    if (w.failed || w.len > SLAB_PAYLOAD_SIZE) { out[0] = 0; return 0; }
    return (int)w.len;
}
bool slab_ble_checksum(const slab_round_t *r, char out[15])
{
    static char payload[SLAB_PAYLOAD_SIZE]; /* serialized task context, not ISR */
    int n = slab_ble_build_payload(r, payload, sizeof(payload));
    if (!n) return false;
    /* Fixed suffix: crc32:xxxxxxxx followed by quote and brace. */
    memcpy(out, payload + n - 16, 14);
    out[14] = 0;
    return true;
}

int slab_ble_build_meta(const slab_round_t *r, int byte_length, int page_length, int long_read,
                        char *out, int out_sz)
{
    if (!out || out_sz < 1) return 0;
    out[0] = 0;
    char sum[15];
    if (!slab_ble_checksum(r, sum) || byte_length < 1 || page_length < 1) return 0;
    int n = snprintf(out, (size_t)out_sz,
                     "{\"schema_version\":2,\"round_id\":\"%s\",\"device_id\":\"%s\","
                     "\"round_sequence\":%lu,\"checksum\":\"%s\",\"byte_length\":%d,"
                     "\"page_length\":%d,\"long_read\":%d}",
                     r->round_id, r->device_id, (unsigned long)r->round_sequence, sum,
                     byte_length, page_length, long_read ? 1 : 0);
    if (n < 0 || n >= out_sz) {
        out[0] = 0;
        return 0;
    }
    return n;
}

static const char *find_key(const char *json, const char *key)
{
    char pat[40];
    int n = snprintf(pat, sizeof(pat), "\"%s\"", key);
    if (n <= 0 || (size_t)n >= sizeof(pat) || !json) return NULL;
    const char *p = strstr(json, pat);
    if (!p) return NULL;
    p += n;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    if (*p != ':') return NULL;
    p++;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    return p;
}

static bool take_string(const char *p, char *out, size_t cap)
{
    if (!p || *p != '"' || cap < 2) return false;
    p++;
    size_t i = 0;
    while (*p && *p != '"') {
        if (i + 1 >= cap) return false;
        out[i++] = *p++;
    }
    if (*p != '"') return false;
    out[i] = 0;
    return i > 0;
}

static bool take_u32(const char *p, uint32_t *out)
{
    if (!p || *p < '0' || *p > '9') return false;
    unsigned long v = 0;
    while (*p >= '0' && *p <= '9') {
        unsigned long next = v * 10ul + (unsigned long)(*p - '0');
        if (next < v || next > 4294967295ul) return false;
        v = next;
        p++;
    }
    if (!v) return false;
    *out = (uint32_t)v;
    return true;
}

bool slab_ble_parse_ack(const char *json, char *device, size_t device_cap, char *round_id,
                        size_t round_cap, uint32_t *sequence, char checksum[15])
{
    if (!json || !device || !round_id || !sequence || !checksum) return false;
    uint32_t schema = 0;
    if (!take_u32(find_key(json, "schema_version"), &schema) || schema != SLAB_SCHEMA_VERSION) return false;
    if (!take_string(find_key(json, "device_id"), device, device_cap)) return false;
    if (!take_string(find_key(json, "round_id"), round_id, round_cap)) return false;
    if (!take_u32(find_key(json, "round_sequence"), sequence)) return false;
    if (!take_string(find_key(json, "checksum"), checksum, 15)) return false;
    return true;
}
