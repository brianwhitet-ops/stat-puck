#include "slab_state.h"
#include "slab_ble.h"
#include "slab_store.h"
#include "slab_crc32.h"
#include "slab_input.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void enter_hole(slab_round_t *r, int strokes, int putts)
{
    for (int i = 0; i < strokes; i++) slab_apply_event(r, SLAB_EVT_PLUS);
    slab_apply_event(r, SLAB_EVT_NEXT);
    if (!putts) slab_apply_event(r, SLAB_EVT_MINUS); /* explicit zero */
    for (int i = 0; i < putts; i++) slab_apply_event(r, SLAB_EVT_PLUS);
    slab_apply_event(r, SLAB_EVT_NEXT);
    slab_apply_event(r, SLAB_EVT_NEXT);
}
static void test_capture(void)
{
    slab_round_t r;
    uint8_t pars[18]; memset(pars, 4, sizeof(pars));
    assert(!slab_round_start(&r, "test", 1, 18, pars));
    char payload[SLAB_PAYLOAD_SIZE];
    for (int i = 0; i < 100; i++) slab_apply_event(&r, SLAB_EVT_NEXT);
    assert(r.current_hole == 1 && r.holes[0].captured == 0);
    assert(r.holes[0].strokes == 0 && r.holes[0].putts == 0 && r.holes[0].fairway == 0);
    assert(slab_round_gir_holes(&r) == 0 && slab_round_fwy_holes(&r) == 0);
    assert(!slab_ble_should_advertise(&r));
    assert(slab_ble_build_payload(&r, payload, sizeof(payload)) > 0);
    assert(strstr(payload, "\"strokes\":null,\"putts\":null,\"drive_result\":null"));
    slab_apply_event(&r, SLAB_EVT_MINUS);
    assert(!r.holes[0].captured && !r.holes[0].strokes);
    slab_apply_event(&r, SLAB_EVT_PLUS);
    slab_apply_event(&r, SLAB_EVT_NEXT);
    slab_apply_event(&r, SLAB_EVT_NEXT);
    assert(r.ui == SLAB_UI_PUTTS_INPUT && r.holes[0].captured == 1);
    slab_apply_event(&r, SLAB_EVT_MINUS);
    assert(r.holes[0].putts == 0 && r.holes[0].captured == 3);
    slab_apply_event(&r, SLAB_EVT_NEXT);
    assert(r.drive_sel == SLAB_FWY_NA);
    slab_apply_event(&r, SLAB_EVT_NEXT);
    assert(r.holes[0].fairway == SLAB_FWY_NA && r.holes[0].captured == 3);
    assert(slab_round_gir_holes(&r) == 1 && slab_round_fwy_holes(&r) == 0);
    assert(r.current_hole == 2 && r.holes[1].captured == 0);
    /* Lowering strokes invalidates previously entered incompatible putts. */
    for (int i = 0; i < 3; i++) slab_apply_event(&r, SLAB_EVT_PLUS);
    slab_apply_event(&r, SLAB_EVT_NEXT);
    for (int i = 0; i < 3; i++) slab_apply_event(&r, SLAB_EVT_PLUS);
    slab_apply_event(&r, SLAB_EVT_BACK);
    slab_apply_event(&r, SLAB_EVT_MINUS);
    assert(r.holes[1].strokes == 2 && r.holes[1].putts == 0 && r.holes[1].captured == 1);
}
static void test_queue(void)
{
    slab_store_t s; slab_store_init(&s);
    for (int round = 0; round < SLAB_QUEUE_CAPACITY; round++) {
        assert(!slab_store_begin(&s, "test-queue", 9, NULL));
        for (int hole = 0; hole < 9; hole++) enter_hole(&s.active, 4, 2);
        assert(slab_round_complete(&s.active));
        assert(!slab_store_finish(&s));
    }
    assert(s.count == 10 && slab_store_valid(&s));
    assert(slab_store_begin(&s, "test-queue", 9, NULL) == SLAB_STORE_FULL);
    char expected[SLAB_PAYLOAD_SIZE], result[SLAB_PAYLOAD_SIZE];
    int n = slab_ble_build_payload(&s.queued[0], expected, sizeof(expected));
    size_t total = 0, offset = 0;
    while (offset < (size_t)n) {
        int got = slab_store_read_chunk(&s, offset, result + offset, 17, &total);
        assert(got > 0 && total == (size_t)n);
        offset += (size_t)got;
    }
    assert(!memcmp(expected, result, (size_t)n) && s.count == 10);
    char retry[17];
    assert(slab_store_read_chunk(&s, 0, retry, sizeof(retry), &total) == 17);
    assert(!memcmp(retry, expected, 17) && s.count == 10);
    char checksum[15]; assert(slab_ble_checksum(&s.queued[0], checksum));
    char id[SLAB_ROUND_ID_SIZE]; strcpy(id, s.queued[0].round_id);
    assert(slab_store_ack(&s, "test-queue", id, 1, "crc32:00000000") == SLAB_STORE_CONFLICT);
    assert(slab_store_ack(&s, "wrong", id, 1, checksum) == SLAB_STORE_CONFLICT);
    assert(!slab_store_ack(&s, "test-queue", id, 1, checksum));
    assert(s.count == 9 && s.queued[0].round_sequence == 2);
    assert(slab_store_ack(&s, "test-queue", id, 1, checksum) == SLAB_STORE_CONFLICT);
    assert(s.count == 9);
    assert(!slab_store_begin(&s, "test-queue", 18, NULL));
    assert(s.active.round_sequence == 11);
    assert(slab_store_read_chunk(&s, 0, retry, sizeof(retry), &total) < 0);
}
static void test_turn_and_ids(void)
{
    slab_round_t r;
    assert(!slab_round_start(&r, "test", 2, 18, NULL));
    assert(!slab_round_finish_nine(&r));
    for (int i = 0; i < 9; i++) enter_hole(&r, 1, 0);
    assert(r.current_hole == 10);
    slab_apply_event(&r, SLAB_EVT_LONG_NEXT);
    assert(r.holes_played == 9 && slab_round_complete(&r));
    assert(!slab_hole_gir_known(&r.holes[0])); /* Unknown course par. */
    slab_round_t next;
    assert(!slab_round_start(&next, "test", 3, 18, NULL));
    assert(strcmp(next.round_id, r.round_id));
    assert(slab_round_start(&next, "bad\"id", 1, 9, NULL));
    assert(slab_round_start(&next, "test", 0, 9, NULL));
    assert(slab_round_start(&next, "test", 1, 10, NULL));
}
static void test_buffers(void)
{
    assert(slab_crc32("123456789", 9) == 0xcbf43926u);
    slab_round_t r; slab_round_init_fixture_complete(&r);
    char full[SLAB_PAYLOAD_SIZE];
    int n = slab_ble_build_payload(&r, full, sizeof(full));
    assert(n > 0);
    for (int cap = 1; cap <= n; cap++) {
        unsigned char out[SLAB_PAYLOAD_SIZE]; memset(out, 0xa5, sizeof(out));
        assert(!slab_ble_build_payload(&r, (char *)out, cap));
        assert(out[0] == 0 && out[cap] == 0xa5);
    }
    assert(slab_ble_build_payload(&r, full, n + 1) == n);
    r.holes_played = 255;
    assert(!slab_ble_build_payload(&r, full, sizeof(full)));
}
static void test_long_press(void)
{
    slab_input_t in; slab_input_init(&in);
    uint8_t raw[SLAB_BTN_COUNT] = {0}; raw[SLAB_BTN_NEXT] = 1;
    int longs = 0, shorts = 0;
    for (int i = 0; i < 180; i++) {
        slab_evt_t ev = slab_input_poll(&in, raw, 10, 0, 0);
        longs += ev == SLAB_EVT_LONG_NEXT; shorts += ev == SLAB_EVT_NEXT;
    }
    raw[SLAB_BTN_NEXT] = 0;
    for (int i = 0; i < 5; i++) {
        slab_evt_t ev = slab_input_poll(&in, raw, 10, 0, 0);
        shorts += ev == SLAB_EVT_NEXT;
    }
    assert(longs == 1 && shorts == 0);
    raw[SLAB_BTN_NEXT] = 1;
    for (int i = 0; i < 8; i++) assert(slab_input_poll(&in, raw, 10, 0, 0) == SLAB_EVT_NONE);
    raw[SLAB_BTN_NEXT] = 0;
    for (int i = 0; i < 8; i++) shorts += slab_input_poll(&in, raw, 10, 0, 0) == SLAB_EVT_NEXT;
    assert(shorts == 1);
}
static void fixtures(const char *dir)
{
    slab_round_t r; char data[SLAB_PAYLOAD_SIZE], path[256];
    const char *names[] = {"untouched", "nine-zero-putts", "eighteen"};
    for (int sample = 0; sample < 3; sample++) {
        if (sample == 0) slab_round_init(&r);
        if (sample == 1) {
            assert(!slab_round_start(&r, "cross-language", 2, 9, NULL));
            for (int i = 0; i < 9; i++) enter_hole(&r, 1, 0);
        }
        if (sample == 2) slab_round_init_fixture_complete(&r);
        assert(slab_ble_build_payload(&r, data, sizeof(data)) > 0);
        snprintf(path, sizeof(path), "%s/%s.json", dir, names[sample]);
        FILE *f = fopen(path, "w"); assert(f); fprintf(f, "%s\n", data); assert(!fclose(f));
    }
}
int main(int argc, char **argv)
{
    test_capture(); test_queue(); test_turn_and_ids(); test_buffers(); test_long_press();
    if (argc == 2) fixtures(argv[1]);
    puts("round v2: capture, zero putts, queue, replay, identities, bounds, turn and long-press tests passed");
    return 0;
}
