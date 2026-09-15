#include "slab_ui.h"

#include "slab_font.h"
#include "slab_frames.h"
#include "slab_state.h"

#include <string.h>

/*
 * Dynamic composer: chrome comes from the Instinct template for the UI
 * state; live fields (strokes, putts, hole, drive, totals, copy) are
 * painted from slab_round_t. When a field equals the value encoded in
 * that state's golden, the template pixels are left alone so host
 * visual stays 0-diff and the 1-bit path can reproduce the thresholded
 * goldens. Any other live value clears the field box and draws digits.
 */

typedef struct {
    uint8_t hole;
    uint8_t par;
    uint8_t strokes;
    uint8_t putts;
    uint8_t drive;
    uint16_t yards;
    int through;
    int vs_through;
    int total;
    int vs_round;
    int gir_hits;
    int gir_holes;
    int gir_pct;
    int putts_total;
    int putts_avg_x100;
} slab_view_t;

/* Instinct-encoded demo values (goldens 01–06). */
#define G_HOLE 7
#define G_PAR 4
#define G_STROKES 4
#define G_PUTTS 2
#define G_DRIVE SLAB_FWY_H
#define G_YARDS 412
#define G_THROUGH 6
#define G_VS_THROUGH 0
#define G_TOTAL 74
#define G_VS_ROUND 2
#define G_GIR_HITS 11
#define G_GIR_HOLES 18
#define G_GIR_PCT 61
#define G_PUTTS_TOT 31
#define G_PUTTS_AVG 172

typedef struct {
    int x, y, w, h;
} slab_box_t;

static const slab_box_t kBoxHole = {46, 6, 16, 12};
static const slab_box_t kBoxPar = {225, 6, 10, 12};
static const slab_box_t kBoxYards = {240, 6, 28, 12};
static const slab_box_t kBoxHeroDefault = {128, 42, 38, 48};
static const slab_box_t kBoxHeroEdit = {128, 48, 36, 46};
static const slab_box_t kBoxVsThrough = {103, 97, 12, 12};
static const slab_box_t kBoxThroughN = {182, 97, 12, 12};
static const slab_box_t kBoxConfirmSt = {10, 50, 12, 14};
static const slab_box_t kBoxConfirmPu = {62, 50, 16, 14};
static const slab_box_t kBoxChipL = {28, 85, 36, 23};
static const slab_box_t kBoxChipH = {66, 85, 66, 23};
static const slab_box_t kBoxChipR = {136, 85, 46, 23};
static const slab_box_t kBoxTotal = {36, 45, 58, 40};
static const slab_box_t kBoxCompleteVs = {48, 94, 28, 16};
static const slab_box_t kBoxStatsTot = {245, 6, 18, 12};
static const slab_box_t kBoxStatsVs = {263, 6, 22, 12};
static const slab_box_t kBoxGirFrac = {20, 61, 40, 14};
static const slab_box_t kBoxGirPct = {56, 61, 84, 34};
static const slab_box_t kBoxPuttsN = {168, 61, 30, 14};
static const slab_box_t kBoxPuttsAvg = {200, 88, 38, 14};

static const slab_hole_t *cur_c(const slab_round_t *r)
{
    int i = (int)r->current_hole - 1;
    if (i < 0) {
        i = 0;
    }
    if (i >= SLAB_HOLES_MAX) {
        i = SLAB_HOLES_MAX - 1;
    }
    return &r->holes[i];
}

static void view_from_round(const slab_round_t *r, slab_view_t *v)
{
    const slab_hole_t *h = cur_c(r);
    memset(v, 0, sizeof(*v));
    v->hole = r->current_hole;
    v->par = h->par;
    v->strokes = h->strokes;
    v->putts = h->putts;
    v->drive = (r->ui == SLAB_UI_END_HOLE_CONFIRM) ? r->drive_sel : h->fairway;
    v->yards = (uint16_t)slab_course_yards((int)r->current_hole);
    v->through = 0;
    for (int i = 0; i < r->holes_played; i++) {
        if (r->holes[i].locked) {
            v->through = i + 1;
        }
    }
    v->vs_through = slab_round_vs_par(r, false);
    v->total = 0;
    for (int i = 0; i < r->holes_played; i++) {
        if (r->holes[i].locked) {
            v->total += r->holes[i].strokes;
        }
    }
    v->vs_round = slab_round_vs_par(r, false);
    v->gir_hits = slab_round_gir_hits(r);
    v->gir_holes = slab_round_gir_holes(r);
    v->gir_pct = v->gir_holes ? (v->gir_hits * 100) / v->gir_holes : 0;
    v->putts_total = slab_round_putts(r);
    v->putts_avg_x100 = v->gir_holes ? (v->putts_total * 100) / v->gir_holes : 0;
}

static int u_to_s(char *b, unsigned n)
{
    char tmp[8];
    int i = 0;
    if (n == 0) {
        b[0] = '0';
        b[1] = 0;
        return 1;
    }
    while (n && i < 7) {
        tmp[i++] = (char)('0' + (n % 10u));
        n /= 10u;
    }
    int j = 0;
    while (i) {
        b[j++] = tmp[--i];
    }
    b[j] = 0;
    return j;
}

static void fmt_hole(char *b, unsigned hole)
{
    b[0] = (char)('0' + ((hole / 10u) % 10u));
    b[1] = (char)('0' + (hole % 10u));
    b[2] = 0;
}

static void fmt_vs(char *b, int v)
{
    if (v == 0) {
        b[0] = 'E';
        b[1] = 0;
        return;
    }
    if (v > 0) {
        b[0] = '+';
        u_to_s(b + 1, (unsigned)v);
        return;
    }
    b[0] = '-';
    u_to_s(b + 1, (unsigned)(-v));
}

static void fmt_frac(char *b, int a, int c)
{
    int n = u_to_s(b, (unsigned)a);
    b[n++] = '/';
    n += u_to_s(b + n, (unsigned)c);
    (void)n;
}

static void fmt_pct(char *b, int pct)
{
    int n = u_to_s(b, (unsigned)pct);
    b[n++] = '%';
    b[n] = 0;
}

static void fmt_avg(char *b, int x100)
{
    int n = u_to_s(b, (unsigned)(x100 / 100));
    b[n++] = '.';
    b[n++] = (char)('0' + ((x100 / 10) % 10));
    b[n++] = (char)('0' + (x100 % 10));
    b[n] = 0;
}

#if SLAB_HOST
static void paint_rgb(slab_fb_t *fb, int x, int y, int ink)
{
    if (ink) {
        slab_fb_set_rgb(fb, x, y, SLAB_INK_R, SLAB_INK_G, SLAB_INK_B);
    } else {
        slab_fb_set_rgb(fb, x, y, SLAB_PAPER_R, SLAB_PAPER_G, SLAB_PAPER_B);
    }
}

static int rgba_luma(const slab_fb_t *fb, int x, int y)
{
    if (x < 0 || x >= SLAB_FB_W || y < 0 || y >= SLAB_FB_H) {
        return 255;
    }
    const uint8_t *p = &fb->rgba[(y * SLAB_FB_W + x) * 4];
    return (p[0] * 30 + p[1] * 59 + p[2] * 11) / 100;
}
#endif

static void erase_box(slab_fb_t *fb, slab_box_t b)
{
    slab_fb_fill_rect(fb, b.x, b.y, b.w, b.h, 0);
#if SLAB_HOST
    slab_fb_fill_rgb(fb, b.x, b.y, b.w, b.h, SLAB_PAPER_R, SLAB_PAPER_G, SLAB_PAPER_B);
#endif
}

static void paint_s(slab_fb_t *fb, int x, int y, const char *s)
{
    int cx = x;
    if (!s) {
        return;
    }
    while (*s) {
        const uint8_t *g = slab_glyph_s(*s);
        for (int row = 0; row < SLAB_FONT_S_H; row++) {
            uint8_t bits = g[row];
            for (int col = 0; col < SLAB_FONT_S_W; col++) {
                if (bits & (uint8_t)(0x80u >> col)) {
                    slab_fb_set(fb, cx + col, y + row, 1);
#if SLAB_HOST
                    paint_rgb(fb, cx + col, y + row, 1);
#endif
                }
            }
        }
        cx += SLAB_FONT_S_ADV;
        s++;
    }
}

static void paint_m_scaled(slab_fb_t *fb, int x, int y, const char *s, int scale)
{
    int cx = x;
    if (!s) {
        return;
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
                            int px = cx + col * scale + dx;
                            int py = y + row * scale + dy;
                            slab_fb_set(fb, px, py, 1);
#if SLAB_HOST
                            paint_rgb(fb, px, py, 1);
#endif
                        }
                    }
                }
            }
        }
        cx += SLAB_FONT_M_ADV * scale;
        s++;
    }
}

static void stamp_text(slab_fb_t *fb, slab_box_t b, const char *s, int scale)
{
    int tw;
    int th;
    erase_box(fb, b);
    if (scale <= 1) {
        tw = slab_text_s_width(s);
        th = SLAB_FONT_S_H;
        paint_s(fb, b.x + (b.w - tw) / 2, b.y + (b.h - th) / 2, s);
    } else {
        tw = slab_text_m_width(s) * scale;
        th = SLAB_FONT_M_H * scale;
        paint_m_scaled(fb, b.x + (b.w - tw) / 2, b.y + (b.h - th) / 2, s, scale);
    }
}

static void stamp_u(slab_fb_t *fb, slab_box_t b, unsigned n, int scale)
{
    char buf[8];
    u_to_s(buf, n);
    stamp_text(fb, b, buf, scale);
}

static void restyle_chip(slab_fb_t *fb, slab_box_t b)
{
    slab_fb_invert_rect(fb, b.x, b.y, b.w, b.h);
#if SLAB_HOST
    for (int y = b.y; y < b.y + b.h; y++) {
        for (int x = b.x; x < b.x + b.w; x++) {
            int ink = rgba_luma(fb, x, y) < SLAB_INK_LUMA;
            paint_rgb(fb, x, y, ink ? 0 : 1);
        }
    }
#endif
}

static void compose_header(slab_fb_t *fb, const slab_view_t *v)
{
    if (v->hole != G_HOLE) {
        char buf[4];
        fmt_hole(buf, v->hole);
        stamp_text(fb, kBoxHole, buf, 1);
    }
    if (v->par != G_PAR) {
        stamp_u(fb, kBoxPar, v->par, 1);
    }
    if (v->yards != G_YARDS) {
        stamp_u(fb, kBoxYards, v->yards, 1);
    }
}

static void compose_drive(slab_fb_t *fb, uint8_t drive)
{
    if (drive == G_DRIVE) {
        return;
    }
    restyle_chip(fb, kBoxChipH);
    if (drive == SLAB_FWY_L) {
        restyle_chip(fb, kBoxChipL);
    } else if (drive == SLAB_FWY_R) {
        restyle_chip(fb, kBoxChipR);
    }
}

static void compose_live(slab_fb_t *fb, slab_ui_t ui, const slab_view_t *v)
{
    char buf[12];

    switch (ui) {
    case SLAB_UI_DEFAULT_HOLE:
        compose_header(fb, v);
        if (v->strokes != G_STROKES) {
            stamp_u(fb, kBoxHeroDefault, v->strokes, 2);
        }
        if (v->vs_through != G_VS_THROUGH) {
            fmt_vs(buf, v->vs_through);
            stamp_text(fb, kBoxVsThrough, buf, 1);
        }
        if (v->through != G_THROUGH) {
            stamp_u(fb, kBoxThroughN, (unsigned)v->through, 1);
        }
        break;

    case SLAB_UI_STROKE_EDIT:
        compose_header(fb, v);
        if (v->strokes != G_STROKES) {
            stamp_u(fb, kBoxHeroEdit, v->strokes, 2);
        }
        break;

    case SLAB_UI_PUTTS_INPUT:
        compose_header(fb, v);
        if (v->putts != G_PUTTS) {
            stamp_u(fb, kBoxHeroEdit, v->putts, 2);
        }
        break;

    case SLAB_UI_END_HOLE_CONFIRM:
        compose_header(fb, v);
        if (v->strokes != G_STROKES) {
            stamp_u(fb, kBoxConfirmSt, v->strokes, 1);
        }
        if (v->putts != G_PUTTS) {
            stamp_u(fb, kBoxConfirmPu, v->putts, 1);
        }
        compose_drive(fb, v->drive);
        break;

    case SLAB_UI_ROUND_COMPLETE_SYNC:
        if (v->total != G_TOTAL) {
            stamp_u(fb, kBoxTotal, (unsigned)v->total, 2);
        }
        if (v->vs_round != G_VS_ROUND) {
            fmt_vs(buf, v->vs_round);
            stamp_text(fb, kBoxCompleteVs, buf, 1);
        }
        break;

    case SLAB_UI_DERIVED_STATS:
        if (v->total != G_TOTAL) {
            stamp_u(fb, kBoxStatsTot, (unsigned)v->total, 1);
        }
        if (v->vs_round != G_VS_ROUND) {
            fmt_vs(buf, v->vs_round);
            stamp_text(fb, kBoxStatsVs, buf, 1);
        }
        if (v->gir_hits != G_GIR_HITS || v->gir_holes != G_GIR_HOLES) {
            fmt_frac(buf, v->gir_hits, v->gir_holes);
            stamp_text(fb, kBoxGirFrac, buf, 1);
        }
        if (v->gir_pct != G_GIR_PCT) {
            fmt_pct(buf, v->gir_pct);
            stamp_text(fb, kBoxGirPct, buf, 2);
        }
        if (v->putts_total != G_PUTTS_TOT) {
            stamp_u(fb, kBoxPuttsN, (unsigned)v->putts_total, 1);
        }
        if (v->putts_avg_x100 != G_PUTTS_AVG) {
            fmt_avg(buf, v->putts_avg_x100);
            stamp_text(fb, kBoxPuttsAvg, buf, 1);
        }
        break;

    default:
        break;
    }
}

void slab_ui_render(slab_fb_t *fb, const slab_round_t *r)
{
    slab_view_t v;

    slab_fb_clear(fb, 0);
    if (!r) {
        return;
    }
    view_from_round(r, &v);
#if SLAB_HOST
    memset(fb->rgba, 0, sizeof(fb->rgba));
    (void)slab_frame_rgba((slab_ui_t)r->ui, fb->rgba);
#endif
    (void)slab_frame_1bit((slab_ui_t)r->ui, fb->px);
    compose_live(fb, (slab_ui_t)r->ui, &v);
}
