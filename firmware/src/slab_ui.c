#include "slab_ui.h"

#include "slab_frames.h"

#include <string.h>

void slab_ui_render(slab_fb_t *fb, const slab_round_t *r)
{
    slab_fb_clear(fb, 0);
    if (!r) {
        return;
    }
#if SLAB_HOST
    memset(fb->rgba, 0, sizeof(fb->rgba));
    (void)slab_frame_rgba((slab_ui_t)r->ui, fb->rgba);
#endif
    /* Real 296x128 1-bit panel buffer — Instinct goldens, thresholded. */
    (void)slab_frame_1bit((slab_ui_t)r->ui, fb->px);
}
