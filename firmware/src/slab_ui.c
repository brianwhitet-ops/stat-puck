#include "slab_ui.h"

#include "slab_frames.h"
#include "slab_state.h"

#include <string.h>

#if SLAB_HOST

void slab_ui_render(slab_fb_t *fb, const slab_round_t *r)
{
    memset(fb->px, 0, sizeof(fb->px));
    memset(fb->rgba, 0, sizeof(fb->rgba));
    if (!r) {
        return;
    }
    if (slab_frame_rgba((slab_ui_t)r->ui, fb->rgba) != 0) {
        return;
    }
    /* 1-bit companion: dark ink → 1. Used only as a panel preview, not goldens. */
    for (int y = 0; y < SLAB_FB_H; y++) {
        for (int x = 0; x < SLAB_FB_W; x++) {
            const uint8_t *p = &fb->rgba[(y * SLAB_FB_W + x) * 4];
            int luma = (p[0] * 30 + p[1] * 59 + p[2] * 11) / 100;
            slab_fb_set(fb, x, y, luma < 140 ? 1 : 0);
        }
    }
}

#else

void slab_ui_render(slab_fb_t *fb, const slab_round_t *r)
{
    /* On-device photos are a later gate. Emulator uses the Instinct RGBA blit. */
    slab_fb_clear(fb, 0);
    (void)r;
}

#endif
