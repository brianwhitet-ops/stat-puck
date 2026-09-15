#include "slab_fb.h"

void slab_fb_clear(slab_fb_t *fb, int ink)
{
    uint8_t v = ink ? 0xFFu : 0x00u;
    for (int i = 0; i < SLAB_FB_BYTES; i++) {
        fb->px[i] = v;
    }
}

static int in_bounds(int x, int y)
{
    return x >= 0 && x < SLAB_FB_W && y >= 0 && y < SLAB_FB_H;
}

void slab_fb_set(slab_fb_t *fb, int x, int y, int ink)
{
    if (!in_bounds(x, y)) {
        return;
    }
    int i = y * SLAB_FB_STRIDE + (x >> 3);
    uint8_t mask = (uint8_t)(0x80u >> (x & 7));
    if (ink) {
        fb->px[i] |= mask;
    } else {
        fb->px[i] &= (uint8_t)~mask;
    }
}

int slab_fb_get(const slab_fb_t *fb, int x, int y)
{
    if (!in_bounds(x, y)) {
        return 0;
    }
    int i = y * SLAB_FB_STRIDE + (x >> 3);
    uint8_t mask = (uint8_t)(0x80u >> (x & 7));
    return (fb->px[i] & mask) ? 1 : 0;
}

void slab_fb_hline(slab_fb_t *fb, int x0, int x1, int y, int ink)
{
    if (x1 < x0) {
        int t = x0;
        x0 = x1;
        x1 = t;
    }
    for (int x = x0; x <= x1; x++) {
        slab_fb_set(fb, x, y, ink);
    }
}

void slab_fb_vline(slab_fb_t *fb, int x, int y0, int y1, int ink)
{
    if (y1 < y0) {
        int t = y0;
        y0 = y1;
        y1 = t;
    }
    for (int y = y0; y <= y1; y++) {
        slab_fb_set(fb, x, y, ink);
    }
}

void slab_fb_fill_rect(slab_fb_t *fb, int x, int y, int w, int h, int ink)
{
    for (int yy = y; yy < y + h; yy++) {
        for (int xx = x; xx < x + w; xx++) {
            slab_fb_set(fb, xx, yy, ink);
        }
    }
}

void slab_fb_rect(slab_fb_t *fb, int x, int y, int w, int h, int ink)
{
    if (w <= 0 || h <= 0) {
        return;
    }
    slab_fb_hline(fb, x, x + w - 1, y, ink);
    slab_fb_hline(fb, x, x + w - 1, y + h - 1, ink);
    slab_fb_vline(fb, x, y, y + h - 1, ink);
    slab_fb_vline(fb, x + w - 1, y, y + h - 1, ink);
}

void slab_fb_invert_rect(slab_fb_t *fb, int x, int y, int w, int h)
{
    for (int yy = y; yy < y + h; yy++) {
        for (int xx = x; xx < x + w; xx++) {
            slab_fb_set(fb, xx, yy, !slab_fb_get(fb, xx, yy));
        }
    }
}
