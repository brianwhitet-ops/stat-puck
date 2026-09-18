#ifndef SLAB_STATE_H
#define SLAB_STATE_H

#include "slab_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void slab_round_init(slab_round_t *r);
/* Caller supplies a stable device identity and durably reserved sequence. */
int slab_round_start(slab_round_t *r, const char *device_id, uint32_t sequence,
                     uint8_t holes, const uint8_t *pars);
bool slab_round_valid(const slab_round_t *r);
bool slab_round_complete(const slab_round_t *r);
bool slab_round_finish_nine(slab_round_t *r);
void slab_round_init_fixture_hole7(slab_round_t *r);
void slab_round_init_fixture_complete(slab_round_t *r);

void slab_hole_apply_defaults(slab_hole_t *h);
bool slab_hole_gir(const slab_hole_t *h);
bool slab_hole_gir_known(const slab_hole_t *h);
int slab_hole_vs_par(const slab_hole_t *h);

int slab_round_vs_par(const slab_round_t *r, bool include_current);
int slab_round_putts(const slab_round_t *r);
int slab_round_putt_holes(const slab_round_t *r);
int slab_round_gir_hits(const slab_round_t *r);
int slab_round_gir_holes(const slab_round_t *r);
int slab_round_fwy_hits(const slab_round_t *r);
int slab_round_fwy_holes(const slab_round_t *r);
int slab_course_yards(int hole); /* 1-based; hole 7 is 412 (golden) */

void slab_sync_ble_gate(slab_round_t *r);

/* Interaction lock: per hole strokes → putts → done. No GIR prompt. */
slab_refresh_t slab_apply_event(slab_round_t *r, slab_evt_t ev);

const char *slab_ui_name(slab_ui_t ui);
char slab_fwy_char(uint8_t fwy);

#ifdef __cplusplus
}
#endif

#endif
