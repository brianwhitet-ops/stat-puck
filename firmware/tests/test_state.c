#include "slab_state.h"
#include "slab_ble.h"
#include "slab_input.h"
#include "slab_persist.h"
#include "slab_epd.h"
#include "slab_ui.h"

#include <stdio.h>
#include <string.h>

static int g_fail;

#define EXPECT(cond, msg)                                                      \
    do {                                                                       \
        if (!(cond)) {                                                         \
            fprintf(stderr, "FAIL %s:%d %s\n", __FILE__, __LINE__, msg);       \
            g_fail++;                                                          \
        }                                                                      \
    } while (0)

static void test_sequence_lock(void)
{
    slab_round_t r;
    slab_round_init_fixture_hole7(&r);
    EXPECT(r.ui == SLAB_UI_DEFAULT_HOLE, "start default");
    EXPECT(!slab_ble_should_advertise(&r), "no mid-round BLE");

    slab_apply_event(&r, SLAB_EVT_NEXT);
    EXPECT(r.ui == SLAB_UI_STROKE_EDIT, "next -> stroke");
    EXPECT(!slab_ble_should_advertise(&r), "no BLE in stroke");

    /* PUTT is not an event; cannot skip. */
    EXPECT(r.ui != SLAB_UI_PUTTS_INPUT, "still in stroke");

    slab_apply_event(&r, SLAB_EVT_NEXT);
    EXPECT(r.ui == SLAB_UI_PUTTS_INPUT, "next -> putts");

    slab_apply_event(&r, SLAB_EVT_NEXT);
    EXPECT(r.ui == SLAB_UI_END_HOLE_CONFIRM, "next -> confirm");
    EXPECT(r.holes[6].par == 4, "hole 7 is par 4");
    EXPECT(r.drive_sel != SLAB_FWY_NA, "drive armed on par 4");
}

static void test_par3_no_drive(void)
{
    slab_round_t r;
    slab_round_init(&r);
    r.current_hole = 3;
    r.ui = SLAB_UI_PUTTS_INPUT;
    slab_apply_event(&r, SLAB_EVT_NEXT);
    EXPECT(r.ui == SLAB_UI_END_HOLE_CONFIRM, "par3 confirm");
    EXPECT(r.drive_sel == SLAB_FWY_NA, "par3 no drive");
    uint8_t before = r.drive_sel;
    slab_apply_event(&r, SLAB_EVT_PLUS);
    EXPECT(r.drive_sel == before, "plus does not cycle drive on par 3");
    slab_apply_event(&r, SLAB_EVT_NEXT);
    EXPECT(r.holes[2].fairway == SLAB_FWY_NA, "locked NA");
    EXPECT(r.holes[2].locked, "hole 3 locked");
}

static void test_gir_derived_only(void)
{
    slab_hole_t h;
    h.par = 4;
    h.strokes = 4;
    h.putts = 2;
    EXPECT(slab_hole_gir(&h), "4-2 <= 2 GIR");
    h.strokes = 5;
    h.putts = 2;
    EXPECT(!slab_hole_gir(&h), "5-2 > 2 no GIR");
    h.par = 3;
    h.strokes = 3;
    h.putts = 2;
    EXPECT(slab_hole_gir(&h), "par3 GIR 3-2 <= 1");
}

static void test_ble_gate(void)
{
    slab_round_t r;
    slab_round_init_fixture_complete(&r);
    EXPECT(r.ui == SLAB_UI_ROUND_COMPLETE_SYNC, "complete");
    EXPECT(slab_ble_should_advertise(&r), "advertise on complete");
    slab_apply_event(&r, SLAB_EVT_MODE);
    EXPECT(r.ui == SLAB_UI_DERIVED_STATS, "stats");
    EXPECT(!slab_ble_should_advertise(&r), "no advertise on stats");
    slab_apply_event(&r, SLAB_EVT_BACK);
    EXPECT(slab_ble_should_advertise(&r), "advertise back on sync");

    char buf[2048];
    int n = slab_ble_build_payload(&r, buf, sizeof(buf));
    EXPECT(n > 40, "payload");
    EXPECT(strstr(buf, "stat-puck.round.v1") != NULL, "schema");
    EXPECT(strstr(buf, "ghin") == NULL && strstr(buf, "GHIN") == NULL, "no GHIN");
}

static void test_persist(void)
{
    slab_round_t a, b;
    slab_round_init_fixture_hole7(&a);
    a.ui = SLAB_UI_STROKE_EDIT;
    a.holes[6].strokes = 6;
    slab_sync_ble_gate(&a);
    slab_persist_set_host_path("/tmp/slab_nvram_test.bin");
    EXPECT(slab_persist_save(&a) == 0, "save");
    memset(&b, 0, sizeof(b));
    EXPECT(slab_persist_load(&b) == 0, "load");
    EXPECT(b.current_hole == 7, "hole persisted");
    EXPECT(b.holes[6].strokes == 6, "strokes persisted");
    EXPECT(b.ui == SLAB_UI_STROKE_EDIT, "ui persisted");
    EXPECT(!slab_ble_should_advertise(&b), "gate after load");
}

static void test_debounce(void)
{
    slab_input_t in;
    slab_input_init(&in);
    uint8_t raw[SLAB_BTN_COUNT] = {0};
    raw[SLAB_BTN_PLUS] = 1;
    EXPECT(slab_input_poll(&in, raw, 5, 0, 0) == SLAB_EVT_NONE, "edge starts timer");
    EXPECT(slab_input_poll(&in, raw, 5, 0, 0) == SLAB_EVT_NONE, "bounce 5ms");
    EXPECT(slab_input_poll(&in, raw, 5, 0, 0) == SLAB_EVT_NONE, "bounce 10ms");
    EXPECT(slab_input_poll(&in, raw, 5, 0, 0) == SLAB_EVT_NONE, "bounce 15ms");
    EXPECT(slab_input_poll(&in, raw, 5, 0, 0) == SLAB_EVT_NONE, "bounce 20ms");
    EXPECT(slab_input_poll(&in, raw, 5, 0, 0) == SLAB_EVT_PLUS, "accept 25ms stable");
    EXPECT(slab_input_poll(&in, raw, 10, 0, 0) == SLAB_EVT_NONE, "no repeat");
}

static void test_epd_policy(void)
{
    slab_epd_t e;
    slab_epd_init(&e);
    EXPECT(slab_epd_choose(&e, SLAB_REFRESH_PARTIAL) == SLAB_REFRESH_PARTIAL,
           "first partial");
    for (int i = 1; i < SLAB_PARTIAL_BEFORE_FULL - 1; i++) {
        EXPECT(slab_epd_choose(&e, SLAB_REFRESH_PARTIAL) == SLAB_REFRESH_PARTIAL,
               "partial streak");
    }
    EXPECT(slab_epd_choose(&e, SLAB_REFRESH_PARTIAL) == SLAB_REFRESH_FULL,
           "ghosting full");
    EXPECT(slab_epd_choose(&e, SLAB_REFRESH_FULL) == SLAB_REFRESH_FULL, "full");
}

static void test_render_size(void)
{
    slab_round_t r;
    slab_fb_t fb;
    slab_round_init_fixture_hole7(&r);
    for (int ui = 0; ui < SLAB_UI_COUNT; ui++) {
        r.ui = (uint8_t)ui;
        slab_sync_ble_gate(&r);
        slab_ui_render(&fb, &r);
        EXPECT(SLAB_FB_W == 296 && SLAB_FB_H == 128, "canvas");
        /* Instinct paper fill ~ (222,223,217) in the interior. */
        const uint8_t *p = &fb.rgba[(64 * SLAB_FB_W + 8) * 4];
        EXPECT(p[0] > 200 && p[1] > 200 && p[2] > 200, "instinct paper");
        EXPECT(p[3] == 255, "opaque interior");
    }
}

int main(void)
{
    test_sequence_lock();
    test_par3_no_drive();
    test_gir_derived_only();
    test_ble_gate();
    test_persist();
    test_debounce();
    test_epd_policy();
    test_render_size();
    if (g_fail) {
        fprintf(stderr, "%d failures\n", g_fail);
        return 1;
    }
    puts("state tests ok");
    return 0;
}
