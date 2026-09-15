#include "slab_font.h"

int slab_text_s_width(const char *s)
{
    int n = 0;
    if (!s) {
        return 0;
    }
    while (*s++) {
        n += SLAB_FONT_S_ADV;
    }
    return n;
}

int slab_text_m_width(const char *s)
{
    int n = 0;
    if (!s) {
        return 0;
    }
    while (*s++) {
        n += SLAB_FONT_M_ADV;
    }
    return n;
}

int slab_text_s(slab_fb_t *fb, int x, int y, const char *s, int ink)
{
    int cx = x;
    if (!s) {
        return 0;
    }
    while (*s) {
        const uint8_t *g = slab_glyph_s(*s);
        for (int row = 0; row < SLAB_FONT_S_H; row++) {
            uint8_t bits = g[row];
            for (int col = 0; col < SLAB_FONT_S_W; col++) {
                if (bits & (uint8_t)(0x80u >> col)) {
                    slab_fb_set(fb, cx + col, y + row, ink);
                }
            }
        }
        cx += SLAB_FONT_S_ADV;
        s++;
    }
    return cx - x;
}

int slab_text_m(slab_fb_t *fb, int x, int y, const char *s, int ink)
{
    int cx = x;
    if (!s) {
        return 0;
    }
    while (*s) {
        const uint8_t *g = slab_glyph_m(*s);
        for (int row = 0; row < SLAB_FONT_M_H; row++) {
            uint8_t b0 = g[row * 2];
            uint8_t b1 = g[row * 2 + 1];
            for (int col = 0; col < SLAB_FONT_M_W; col++) {
                uint8_t bit = (col < 8) ? (b0 & (uint8_t)(0x80u >> col))
                                        : (b1 & (uint8_t)(0x80u >> (col - 8)));
                if (bit) {
                    slab_fb_set(fb, cx + col, y + row, ink);
                }
            }
        }
        cx += SLAB_FONT_M_ADV;
        s++;
    }
    return cx - x;
}

int slab_text_m_scaled(slab_fb_t *fb, int x, int y, const char *s, int ink, int scale)
{
    int cx = x;
    if (!s) {
        return 0;
    }
    if (scale < 1) {
        scale = 1;
    }
    while (*s) {
        const uint8_t *g = slab_glyph_m(*s);
        for (int row = 0; row < SLAB_FONT_M_H; row++) {
            uint8_t b0 = g[row * 2];
            uint8_t b1 = g[row * 2 + 1];
            for (int col = 0; col < SLAB_FONT_M_W; col++) {
                uint8_t bit = (col < 8) ? (b0 & (uint8_t)(0x80u >> col))
                                        : (b1 & (uint8_t)(0x80u >> (col - 8)));
                if (bit) {
                    for (int dy = 0; dy < scale; dy++) {
                        for (int dx = 0; dx < scale; dx++) {
                            slab_fb_set(fb, cx + col * scale + dx, y + row * scale + dy,
                                        ink);
                        }
                    }
                }
            }
        }
        cx += SLAB_FONT_M_ADV * scale;
        s++;
    }
    return cx - x;
}
