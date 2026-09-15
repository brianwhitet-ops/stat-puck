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

static int px_diff(const slab_fb_t *a, const slab_fb_t *b)
{
    int n = 0;
    for (int y = 0; y < SLAB_FB_H; y++) {
        for (int x = 0; x < SLAB_FB_W; x++) {
            n += slab_fb_get(a, x, y) != slab_fb_get(b, x, y);
        }
    }
    return n;
}

static void render(slab_fb_t *fb, slab_round_t *r)
{
    slab_sync_ble_gate(r);
    memset(fb, 0xFF, sizeof(*fb));
    slab_ui_render(fb, r);
}

static int expect_change(const char *label, const slab_fb_t *a, const slab_fb_t *b)
{
    int d = px_diff(a, b);
    int same = memcmp(a->px, b->px, SLAB_FB_BYTES) == 0;
    printf("device pixels  %s  diff=%d\n", label, d);
    if (same || d <= 0) {
        fprintf(stderr, "FAIL %s: packed buffer identical (static blit?)\n", label);
        g_fail++;
        return 0;
    }
    return d;
}

int main(void)
{
    slab_round_t r, r2;
    slab_fb_t a, b, tmpl;
    uint8_t raw[SLAB_FB_BYTES];

    EXPECT(sizeof(a.px) == SLAB_FB_BYTES, "packed 1-bit size");
    EXPECT(sizeof(a) == SLAB_FB_BYTES, "no RGBA on device fb");

    slab_round_init_fixture_hole7(&r);
    EXPECT(r.holes[6].strokes == 4, "fixture strokes 4 (golden)");
    EXPECT(r.holes[6].putts == 2, "fixture putts 2 (golden)");
    EXPECT(r.drive_sel == SLAB_FWY_H, "fixture fairway (golden)");

    for (int ui = 0; ui < SLAB_UI_COUNT; ui++) {
        r.ui = (uint8_t)ui;
        render(&a, &r);
        int ink = count_ink(&a);
        EXPECT(ink > 400, "panel not blank");
        EXPECT(ink < SLAB_FB_W * SLAB_FB_H / 2, "not inverted flood");
    }

    /* Chrome template alone (enum blit) vs live compose with a different score. */
    EXPECT(slab_frame_1bit(SLAB_UI_DEFAULT_HOLE, raw) == 0, "template");
    memcpy(tmpl.px, raw, SLAB_FB_BYTES);

    slab_round_init_fixture_hole7(&r);
    r.ui = SLAB_UI_DEFAULT_HOLE;
    render(&a, &r);
    /* Fixture values reproduce the thresholded golden (no field patch). */
    EXPECT(memcmp(a.px, tmpl.px, SLAB_FB_BYTES) == 0,
           "fixture 1-bit reproduces golden template");

    r.holes[6].strokes = 7;
    render(&b, &r);
    expect_change("DEFAULT strokes 4 vs 7", &a, &b);
    EXPECT(memcmp(b.px, tmpl.px, SLAB_FB_BYTES) != 0,
           "live strokes=7 is not the static template");

    slab_round_init_fixture_hole7(&r2);
    r2.ui = SLAB_UI_DEFAULT_HOLE;
    r2.holes[6].strokes = 9;
    render(&a, &r2);
    expect_change("DEFAULT strokes 7 vs 9", &b, &a);

    slab_round_init_fixture_hole7(&r);
    r.ui = SLAB_UI_PUTTS_INPUT;
    render(&a, &r);
    r.holes[6].putts = 0;
    render(&b, &r);
    expect_change("PUTTS 2 vs 0", &a, &b);

    slab_round_init_fixture_hole7(&r);
    r.ui = SLAB_UI_DEFAULT_HOLE;
    render(&a, &r);
    r.current_hole = 18;
    render(&b, &r);
    expect_change("hole 7 vs 18", &a, &b);

    slab_round_init_fixture_hole7(&r);
    r.ui = SLAB_UI_END_HOLE_CONFIRM;
    r.drive_sel = SLAB_FWY_H;
    render(&a, &r);
    r.drive_sel = SLAB_FWY_L;
    render(&b, &r);
    expect_change("drive FAIRWAY vs LEFT", &a, &b);
    r.drive_sel = SLAB_FWY_R;
    render(&a, &r);
    expect_change("drive LEFT vs RIGHT", &b, &a);

    slab_round_init_fixture_complete(&r);
    r.ui = SLAB_UI_ROUND_COMPLETE_SYNC;
    render(&a, &r);
    r.holes[17].strokes = 12;
    render(&b, &r);
    expect_change("complete total 74 vs 80", &a, &b);

    slab_round_init_fixture_complete(&r);
    r.ui = SLAB_UI_DERIVED_STATS;
    render(&a, &r);
    r.holes[17].putts = 6;
    render(&b, &r);
    expect_change("stats putts 31 vs 35", &a, &b);

    if (g_fail) {
        fprintf(stderr, "%d device-1bit failures\n", g_fail);
        return 1;
    }
    puts("device 1-bit path ok (SLAB_HOST=0, live pixels change)");
    return 0;
}
