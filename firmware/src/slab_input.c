#include "slab_input.h"

void slab_input_init(slab_input_t *in)
{
    for (int i = 0; i < SLAB_BTN_COUNT; i++) {
        in->stable[i] = 0;
        in->last[i] = 0;
        in->held_ms[i] = 0;
    }
    in->enc_a = 0;
    in->enc_b = 0;
    in->enc_accum = 0;
}

static slab_evt_t from_plus_minus(int plus, int minus)
{
    if (plus) {
        return SLAB_EVT_PLUS;
    }
    if (minus) {
        return SLAB_EVT_MINUS;
    }
    return SLAB_EVT_NONE;
}

slab_evt_t slab_input_poll(slab_input_t *in, const uint8_t raw_down[SLAB_BTN_COUNT],
                           uint16_t dt_ms, uint8_t enc_a, uint8_t enc_b)
{
    slab_evt_t ev = SLAB_EVT_NONE;

    /* Pad debounce: require stable SLAB_DEBOUNCE_MS, emit on rising edge. */
    for (int i = 0; i < SLAB_BTN_COUNT; i++) {
        uint8_t down = raw_down[i] ? 1 : 0;
        if (down == in->last[i]) {
            if (in->held_ms[i] < 60000) {
                in->held_ms[i] = (uint16_t)(in->held_ms[i] + dt_ms);
            }
        } else {
            in->last[i] = down;
            in->held_ms[i] = 0;
        }
        if (in->held_ms[i] >= SLAB_DEBOUNCE_MS && in->stable[i] != down) {
            uint8_t prev = in->stable[i];
            in->stable[i] = down;
            if (down && !prev) {
                if (i == SLAB_BTN_PLUS && ev == SLAB_EVT_NONE) {
                    ev = SLAB_EVT_PLUS;
                } else if (i == SLAB_BTN_MINUS && ev == SLAB_EVT_NONE) {
                    ev = SLAB_EVT_MINUS;
                } else if (i == SLAB_BTN_NEXT && ev == SLAB_EVT_NONE) {
                    ev = SLAB_EVT_NEXT;
                } else if (i == SLAB_BTN_MODE && ev == SLAB_EVT_NONE) {
                    ev = SLAB_EVT_BACK;
                }
                /* PUTT ignored: sequential lock, no skip to putts / no GIR. */
            }
        }
        if (i == SLAB_BTN_NEXT && in->stable[i] &&
            in->held_ms[i] >= SLAB_LONG_PRESS_MS) {
            in->held_ms[i] = 0;
            ev = SLAB_EVT_LONG_NEXT;
        }
    }

    /* Quadrature dial → same +/- path. 2-bit Gray: 00 01 11 10. */
    uint8_t prev = (uint8_t)((in->enc_a << 1) | in->enc_b);
    uint8_t now = (uint8_t)((enc_a << 1) | enc_b);
    if (now != prev) {
        int8_t dir = 0;
        if ((prev == 0 && now == 1) || (prev == 1 && now == 3) ||
            (prev == 3 && now == 2) || (prev == 2 && now == 0)) {
            dir = +1;
        } else if ((prev == 0 && now == 2) || (prev == 2 && now == 3) ||
                   (prev == 3 && now == 1) || (prev == 1 && now == 0)) {
            dir = -1;
        }
        in->enc_accum = (int8_t)(in->enc_accum + dir);
        in->enc_a = enc_a;
        in->enc_b = enc_b;
        if (in->enc_accum >= 4) {
            in->enc_accum = 0;
            if (ev == SLAB_EVT_NONE) {
                ev = SLAB_EVT_PLUS;
            }
        } else if (in->enc_accum <= -4) {
            in->enc_accum = 0;
            if (ev == SLAB_EVT_NONE) {
                ev = SLAB_EVT_MINUS;
            }
        }
    }

    (void)from_plus_minus;
    return ev;
}
