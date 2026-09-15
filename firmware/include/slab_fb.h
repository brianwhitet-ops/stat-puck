#ifndef SLAB_FB_FRAME_H
#define SLAB_FB_FRAME_H

#include <stdint.h>
#include <stdbool.h>
#include "slab_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t px[SLAB_FB_BYTES];
#if SLAB_HOST
    uint8_t rgba[SLAB_FB_W * SLAB_FB_H * 4];
#endif
} slab_fb_t;

void slab_fb_clear(slab_fb_t *fb, int ink);
void slab_fb_set(slab_fb_t *fb, int x, int y, int ink);
int slab_fb_get(const slab_fb_t *fb, int x, int y);
void slab_fb_hline(slab_fb_t *fb, int x0, int x1, int y, int ink);
void slab_fb_vline(slab_fb_t *fb, int x, int y0, int y1, int ink);
void slab_fb_fill_rect(slab_fb_t *fb, int x, int y, int w, int h, int ink);
void slab_fb_rect(slab_fb_t *fb, int x, int y, int w, int h, int ink);
void slab_fb_invert_rect(slab_fb_t *fb, int x, int y, int w, int h);

#if SLAB_HOST
void slab_fb_set_rgb(slab_fb_t *fb, int x, int y, uint8_t r, uint8_t g, uint8_t b);
void slab_fb_fill_rgb(slab_fb_t *fb, int x, int y, int w, int h, uint8_t r, uint8_t g,
                      uint8_t b);
#endif

#ifdef __cplusplus
}
#endif

#endif
