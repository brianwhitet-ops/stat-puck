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
