#include "slab_epd.h"

void slab_epd_init(slab_epd_t *epd)
{
    epd->partials_since_full = 0;
}

slab_refresh_t slab_epd_choose(slab_epd_t *epd, slab_refresh_t hint)
{
    if (hint == SLAB_REFRESH_NONE) {
        return SLAB_REFRESH_NONE;
    }
    if (hint == SLAB_REFRESH_FULL) {
        epd->partials_since_full = 0;
        return SLAB_REFRESH_FULL;
    }
    epd->partials_since_full++;
    if (epd->partials_since_full >= SLAB_PARTIAL_BEFORE_FULL) {
        epd->partials_since_full = 0;
        return SLAB_REFRESH_FULL;
    }
    return SLAB_REFRESH_PARTIAL;
}
