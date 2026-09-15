#ifndef SLAB_INPUT_H
#define SLAB_INPUT_H

#include "slab_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t stable[SLAB_BTN_COUNT];
    uint8_t last[SLAB_BTN_COUNT];
    uint16_t held_ms[SLAB_BTN_COUNT];
    uint8_t enc_a;
    uint8_t enc_b;
    int8_t enc_accum;
} slab_input_t;

void slab_input_init(slab_input_t *in);

/*
 * raw_down[i] = 1 when pad i is pressed (already inverted from pull-up).
 * dt_ms is the poll interval. Encoder A/B are optional (pass 0,0 if unused).
 */
slab_evt_t slab_input_poll(slab_input_t *in, const uint8_t raw_down[SLAB_BTN_COUNT],
                           uint16_t dt_ms, uint8_t enc_a, uint8_t enc_b);

#ifdef __cplusplus
}
#endif

#endif
