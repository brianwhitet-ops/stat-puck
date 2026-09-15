#include "slab_state.h"
#include "slab_ui.h"
#include "slab_frames.h"

#include <stdio.h>
#include <string.h>

/* Compiled with -DSLAB_HOST=0 — same sources as the XIAO/non-host build. */

static int g_fail;

#define EXPECT(cond, msg)                                                      \
    do {                                                                       \
        if (!(cond)) {                                                         \
            fprintf(stderr, "FAIL %s:%d %s\n", __FILE__, __LINE__, msg);       \
            g_fail++;                                                          \
        }                                                                      \
    } while (0)

static int count_ink(const slab_fb_t *fb)
{
    int n = 0;
    for (int y = 0; y < SLAB_FB_H; y++) {
        for (int x = 0; x < SLAB_FB_W; x++) {
            n += slab_fb_get(fb, x, y);
        }
    }
    return n;
}

int main(void)
{
    slab_round_t r;
    slab_fb_t fb;
    slab_round_init_fixture_hole7(&r);

    EXPECT(sizeof(fb.px) == SLAB_FB_BYTES, "packed 1-bit size");
    EXPECT(sizeof(fb) == SLAB_FB_BYTES, "no RGBA on device fb");

    for (int ui = 0; ui < SLAB_UI_COUNT; ui++) {
        r.ui = (uint8_t)ui;
        slab_sync_ble_gate(&r);
        memset(&fb, 0xFF, sizeof(fb));
        slab_ui_render(&fb, &r);
        int ink = count_ink(&fb);
        EXPECT(ink > 400, "panel not blank");
        EXPECT(ink < SLAB_FB_W * SLAB_FB_H / 2, "not inverted flood");
        EXPECT(slab_frame_1bit((slab_ui_t)ui, fb.px) == 0, "1bit copy");
    }

    if (g_fail) {
        fprintf(stderr, "%d device-1bit failures\n", g_fail);
        return 1;
    }
    puts("device 1-bit path ok (SLAB_HOST=0)");
    return 0;
}
