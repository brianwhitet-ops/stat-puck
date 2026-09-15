#ifndef SLAB_TYPES_H
#define SLAB_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include "slab_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SLAB_UI_DEFAULT_HOLE = 0,
    SLAB_UI_STROKE_EDIT,
    SLAB_UI_PUTTS_INPUT,
    SLAB_UI_END_HOLE_CONFIRM,
    SLAB_UI_ROUND_COMPLETE_SYNC,
    SLAB_UI_DERIVED_STATS,
    SLAB_UI_COUNT
} slab_ui_t;

typedef enum {
    SLAB_FWY_NA = 0, /* par 3, or unset */
    SLAB_FWY_L,
    SLAB_FWY_H, /* fairway / hit */
    SLAB_FWY_R
} slab_fwy_t;

typedef enum {
    SLAB_BTN_PLUS = 0,
    SLAB_BTN_MINUS,
    SLAB_BTN_PUTT,
    SLAB_BTN_MODE,
    SLAB_BTN_NEXT,
    SLAB_BTN_COUNT
} slab_btn_t;

typedef enum {
    SLAB_EVT_NONE = 0,
    SLAB_EVT_PLUS,
    SLAB_EVT_MINUS,
    SLAB_EVT_NEXT,
    SLAB_EVT_BACK,
    SLAB_EVT_MODE,
    SLAB_EVT_LONG_NEXT
} slab_evt_t;

typedef enum {
    SLAB_REFRESH_NONE = 0,
    SLAB_REFRESH_PARTIAL,
    SLAB_REFRESH_FULL
} slab_refresh_t;

typedef struct {
    uint8_t hole;    /* 1..18 */
    uint8_t par;
    uint8_t strokes;
    uint8_t putts;
    uint8_t fairway; /* slab_fwy_t */
    uint8_t locked;  /* 1 after END_HOLE_CONFIRM */
} slab_hole_t;

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint8_t holes_played; /* 9 or 18 */
    uint8_t current_hole; /* 1-based */
    uint8_t ui;
    uint8_t drive_sel; /* slab_fwy_t while confirming */
    uint8_t synced;
    uint8_t ble_advertise; /* 1 only in ROUND_COMPLETE_SYNC */
    char round_id[37];
    char course_name[32];
    char tees[12];
    slab_hole_t holes[SLAB_HOLES_MAX];
} slab_round_t;

#ifdef __cplusplus
}
#endif

#endif
