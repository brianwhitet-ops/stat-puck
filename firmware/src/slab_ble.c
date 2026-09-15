#include "slab_ble.h"

#include "slab_state.h"

#include <stdio.h>
#include <stddef.h>

bool slab_ble_should_advertise(const slab_round_t *r)
{
    return r && r->ui == SLAB_UI_ROUND_COMPLETE_SYNC && r->ble_advertise;
}

static const char *fwy_json(uint8_t f)
{
    switch (f) {
    case SLAB_FWY_L:
        return "L";
    case SLAB_FWY_H:
        return "H";
    case SLAB_FWY_R:
        return "R";
    default:
        return "NA";
    }
}

int slab_ble_build_payload(const slab_round_t *r, char *out, int out_sz)
{
    if (!r || !out || out_sz < 8) {
        return 0;
    }
    int n = 0;
#define APP(fmt, ...)                                                          \
    do {                                                                       \
        int _w = snprintf(out + n, (n < out_sz) ? (size_t)(out_sz - n) : 0,    \
                          fmt, ##__VA_ARGS__);                                 \
        if (_w > 0) {                                                          \
            n += _w;                                                           \
        }                                                                      \
    } while (0)

    APP("{\"schema\":\"stat-puck.round.v1\",\"round_id\":\"%s\",\"device_id\":\"%s\",",
        r->round_id, SLAB_DEVICE_ID);
    APP("\"course\":{\"name\":\"%s\",\"tees\":\"%s\",\"course_id\":\"local:example\"},",
        r->course_name, r->tees);
    APP("\"holes_played\":%u,\"score_type\":\"Home\",\"holes\":[",
        (unsigned)r->holes_played);

    for (int i = 0; i < r->holes_played; i++) {
        const slab_hole_t *h = &r->holes[i];
        APP("%s{\"hole\":%u,\"par\":%u,\"strokes\":%u,\"putts\":%u,\"fairway\":\"%s\",\"gir\":%s}",
            (i ? "," : ""), (unsigned)h->hole, (unsigned)h->par,
            (unsigned)h->strokes, (unsigned)h->putts, fwy_json(h->fairway),
            slab_hole_gir(h) ? "true" : "false");
    }
    APP("]}");
#undef APP
    if (n >= out_sz) {
        out[out_sz - 1] = 0;
        return out_sz - 1;
    }
    return n;
}
