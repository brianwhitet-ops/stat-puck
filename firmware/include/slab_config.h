#ifndef SLAB_CONFIG_H
#define SLAB_CONFIG_H

/* Waveshare 2.9" B/W module, landscape. Native panel is 128x296. */
#define SLAB_FB_W 296
#define SLAB_FB_H 128
#define SLAB_FB_STRIDE 37 /* 296 / 8 */
#define SLAB_FB_BYTES (SLAB_FB_STRIDE * SLAB_FB_H)

#define SLAB_HOLES_MAX 18
#define SLAB_HOLES_DEFAULT 18

#define SLAB_DEBOUNCE_MS 25
#define SLAB_LONG_PRESS_MS 600
#define SLAB_ENCODER_DEBOUNCE_MS 2

#define SLAB_PARTIAL_BEFORE_FULL 8
#define SLAB_REFRESH_FULL_MS 3000
#define SLAB_REFRESH_PARTIAL_MS 600
#define SLAB_REFRESH_PARTIAL_TYP_MS 300
#define SLAB_BUSY_TIMEOUT_MS 5000

#define SLAB_STROKES_MIN 1
#define SLAB_STROKES_MAX 15
#define SLAB_PUTTS_MIN 0
#define SLAB_PUTTS_MAX 10

#define SLAB_PERSIST_MAGIC 0x534C4231u /* 'SLB1' */
#define SLAB_PERSIST_VERSION 1

#define SLAB_DEVICE_ID "SPK-0001"

/* Instinct paper / ink (host RGBA patch). Luma < SLAB_INK_LUMA is ink. */
#define SLAB_PAPER_R 222
#define SLAB_PAPER_G 223
#define SLAB_PAPER_B 217
#define SLAB_INK_R 23
#define SLAB_INK_G 24
#define SLAB_INK_B 22
#define SLAB_INK_LUMA 140

#endif
