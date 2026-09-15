#ifndef SLAB_FONT_H
#define SLAB_FONT_H

#include <stdint.h>
#include "slab_fb.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SLAB_FONT_S_W 7
#define SLAB_FONT_S_H 12
#define SLAB_FONT_S_ADV 8

#define SLAB_FONT_M_W 10
#define SLAB_FONT_M_H 16
#define SLAB_FONT_M_ADV 11
#define SLAB_FONT_M_STRIDE 2

const uint8_t *slab_glyph_s(char c);
const uint8_t *slab_glyph_m(char c);

int slab_text_s(slab_fb_t *fb, int x, int y, const char *s, int ink);
int slab_text_m(slab_fb_t *fb, int x, int y, const char *s, int ink);
int slab_text_m_scaled(slab_fb_t *fb, int x, int y, const char *s, int ink, int scale);
int slab_text_s_width(const char *s);
int slab_text_m_width(const char *s);

#ifdef __cplusplus
}
#endif

#endif
