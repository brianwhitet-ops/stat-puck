#ifndef SLAB_FRAMES_H
#define SLAB_FRAMES_H

#include "slab_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#if SLAB_HOST
/* Host/emulator only: decompressed Instinct RGBA (296*128*4). */
int slab_frame_rgba(slab_ui_t ui, uint8_t *rgba);
#endif

/* Device + host: packed 1-bit panel buffer (SLAB_FB_BYTES), ink=1. */
int slab_frame_1bit(slab_ui_t ui, uint8_t *packed);

#ifdef __cplusplus
}
#endif

#endif
