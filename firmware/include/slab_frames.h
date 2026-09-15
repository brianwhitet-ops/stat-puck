#ifndef SLAB_FRAMES_H
#define SLAB_FRAMES_H

#include "slab_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Decompress the Instinct-locked 296x128 RGBA frame for ui into rgba[296*128*4]. */
int slab_frame_rgba(slab_ui_t ui, uint8_t *rgba);

#ifdef __cplusplus
}
#endif

#endif
