#ifndef SLAB_UI_H
#define SLAB_UI_H

#include "slab_types.h"
#include "slab_fb.h"

#ifdef __cplusplus
extern "C" {
#endif

void slab_ui_render(slab_fb_t *fb, const slab_round_t *r);

#ifdef __cplusplus
}
#endif

#endif
