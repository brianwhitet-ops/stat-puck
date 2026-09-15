#ifndef SLAB_EPD_H
#define SLAB_EPD_H

#include "slab_types.h"
#include "slab_fb.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Panel lock (P1):
 *   Waveshare 2.9" B/W module V2/V3, 296x128
 *   Controller: SSD1680 (Good Display GDEM029T94 / GDEY029T94 family)
 *   Host driver: GxEPD2_290_T94_V2  (partial LUT written to registers)
 *   V1 fallback only: IL3820/SSD1608 via GxEPD2_290 — not the P1 lock
 *
 * Refresh policy:
 *   PARTIAL (~0.3 s typ / 0.6 s listed) on in-place stroke/putt value edits
 *   FULL    (~3 s) on UI state change, hole lock, round complete, every
 *           SLAB_PARTIAL_BEFORE_FULL partials (ghosting)
 *   Wait BUSY (module: high=busy) with SLAB_BUSY_TIMEOUT_MS
 *   Sleep panel between refreshes
 */
typedef struct {
    uint8_t partials_since_full;
} slab_epd_t;

void slab_epd_init(slab_epd_t *epd);
slab_refresh_t slab_epd_choose(slab_epd_t *epd, slab_refresh_t hint);

#ifdef __cplusplus
}
#endif

#endif
