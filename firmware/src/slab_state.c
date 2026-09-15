#include "slab_state.h"

#include <string.h>

static const uint8_t kDefaultPars[SLAB_HOLES_MAX] = {
    4, 4, 3, 5, 4, 4, 4, 3, 5,
    4, 4, 3, 5, 4, 4, 3, 4, 5
};

static void fill_round_id(char *dst)
{
    /* Deterministic local id — not a GHIN number. */
    memcpy(dst, "00000000-0000-4000-8000-000000000001", 37);
}

void slab_hole_apply_defaults(slab_hole_t *h)
{
    h->strokes = h->par;
    h->putts = 2;
    h->fairway = (h->par == 3) ? SLAB_FWY_NA : SLAB_FWY_H;
    h->locked = 0;
}

static void init_holes(slab_round_t *r)
{
    for (int i = 0; i < SLAB_HOLES_MAX; i++) {
        r->holes[i].hole = (uint8_t)(i + 1);
        r->holes[i].par = kDefaultPars[i];
        slab_hole_apply_defaults(&r->holes[i]);
    }
}

void slab_round_init(slab_round_t *r)
{
    memset(r, 0, sizeof(*r));
    r->magic = SLAB_PERSIST_MAGIC;
    r->version = SLAB_PERSIST_VERSION;
    r->holes_played = SLAB_HOLES_DEFAULT;
    r->current_hole = 1;
    r->ui = SLAB_UI_DEFAULT_HOLE;
    r->drive_sel = SLAB_FWY_H;
    memcpy(r->course_name, "LOCAL EXAMPLE", 14);
    memcpy(r->tees, "BLUE", 5);
    fill_round_id(r->round_id);
    init_holes(r);
    slab_sync_ble_gate(r);
}

static void lock_played(slab_round_t *r, int last_inclusive, const uint8_t *strokes,
                        const uint8_t *putts, const uint8_t *fwy)
{
    for (int i = 0; i < last_inclusive; i++) {
        r->holes[i].strokes = strokes[i];
        r->holes[i].putts = putts[i];
        r->holes[i].fairway = (r->holes[i].par == 3) ? SLAB_FWY_NA : fwy[i];
        r->holes[i].locked = 1;
    }
}

void slab_round_init_fixture_hole7(slab_round_t *r)
{
    slab_round_init(r);
    static const uint8_t st[] = {5, 4, 3, 5, 4, 6};
    static const uint8_t pu[] = {2, 2, 2, 2, 1, 2};
    static const uint8_t fw[] = {SLAB_FWY_L, SLAB_FWY_H, SLAB_FWY_NA,
                                 SLAB_FWY_H, SLAB_FWY_R, SLAB_FWY_L};
    lock_played(r, 6, st, pu, fw);
    r->current_hole = 7;
    r->holes[6].strokes = 5;
    r->holes[6].putts = 2;
    r->holes[6].fairway = SLAB_FWY_L;
    r->drive_sel = SLAB_FWY_L;
    r->ui = SLAB_UI_DEFAULT_HOLE;
    slab_sync_ble_gate(r);
}

void slab_round_init_fixture_complete(slab_round_t *r)
{
    slab_round_init_fixture_hole7(r);
    static const uint8_t st[] = {5, 4, 3, 5, 4, 6, 5, 3, 5, 4, 5, 3, 6, 4, 4, 3, 4, 5};
    static const uint8_t pu[] = {2, 2, 2, 2, 1, 2, 2, 1, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2};
    static const uint8_t fw[] = {
        SLAB_FWY_L, SLAB_FWY_H, SLAB_FWY_NA, SLAB_FWY_H, SLAB_FWY_R, SLAB_FWY_L,
        SLAB_FWY_L, SLAB_FWY_NA, SLAB_FWY_H, SLAB_FWY_H, SLAB_FWY_L, SLAB_FWY_NA,
        SLAB_FWY_R, SLAB_FWY_H, SLAB_FWY_H, SLAB_FWY_NA, SLAB_FWY_H, SLAB_FWY_H
    };
    lock_played(r, 18, st, pu, fw);
    r->current_hole = 18;
    r->ui = SLAB_UI_ROUND_COMPLETE_SYNC;
    r->holes_played = 18;
    slab_sync_ble_gate(r);
}

bool slab_hole_gir(const slab_hole_t *h)
{
    /* GIR derived only: (strokes − putts) ≤ (par − 2). Never prompted. */
    int approach = (int)h->strokes - (int)h->putts;
    int need = (int)h->par - 2;
    return approach <= need;
}

int slab_hole_vs_par(const slab_hole_t *h)
{
    return (int)h->strokes - (int)h->par;
}

int slab_round_vs_par(const slab_round_t *r, bool include_current)
{
    int sum = 0;
    int cur = r->current_hole;
    for (int i = 0; i < r->holes_played; i++) {
        int hole = i + 1;
        if (r->holes[i].locked || (include_current && hole == cur)) {
            sum += slab_hole_vs_par(&r->holes[i]);
        }
    }
    return sum;
}

int slab_round_putts(const slab_round_t *r)
{
    int n = 0;
    for (int i = 0; i < r->holes_played; i++) {
        if (r->holes[i].locked) {
            n += r->holes[i].putts;
        }
    }
    return n;
}

int slab_round_gir_hits(const slab_round_t *r)
{
    int n = 0;
    for (int i = 0; i < r->holes_played; i++) {
        if (r->holes[i].locked && slab_hole_gir(&r->holes[i])) {
            n++;
        }
    }
    return n;
}

int slab_round_gir_holes(const slab_round_t *r)
{
    int n = 0;
    for (int i = 0; i < r->holes_played; i++) {
        if (r->holes[i].locked) {
            n++;
        }
    }
    return n;
}

int slab_round_fwy_hits(const slab_round_t *r)
{
    int n = 0;
    for (int i = 0; i < r->holes_played; i++) {
        if (r->holes[i].locked && r->holes[i].par != 3 &&
            r->holes[i].fairway == SLAB_FWY_H) {
            n++;
        }
    }
    return n;
}

int slab_round_fwy_holes(const slab_round_t *r)
{
    int n = 0;
    for (int i = 0; i < r->holes_played; i++) {
        if (r->holes[i].locked && r->holes[i].par != 3) {
            n++;
        }
    }
    return n;
}

void slab_sync_ble_gate(slab_round_t *r)
{
    r->ble_advertise = (r->ui == SLAB_UI_ROUND_COMPLETE_SYNC) ? 1 : 0;
}

const char *slab_ui_name(slab_ui_t ui)
{
    switch (ui) {
    case SLAB_UI_DEFAULT_HOLE:
        return "DEFAULT_HOLE";
    case SLAB_UI_STROKE_EDIT:
        return "STROKE_EDIT";
    case SLAB_UI_PUTTS_INPUT:
        return "PUTTS_INPUT";
    case SLAB_UI_END_HOLE_CONFIRM:
        return "END_HOLE_CONFIRM";
    case SLAB_UI_ROUND_COMPLETE_SYNC:
        return "ROUND_COMPLETE_SYNC";
    case SLAB_UI_DERIVED_STATS:
        return "DERIVED_STATS";
    default:
        return "UNKNOWN";
    }
}

char slab_fwy_char(uint8_t fwy)
{
    switch (fwy) {
    case SLAB_FWY_L:
        return 'L';
    case SLAB_FWY_H:
        return 'H';
    case SLAB_FWY_R:
        return 'R';
    default:
        return 'N';
    }
}

static slab_hole_t *cur(slab_round_t *r)
{
    int i = (int)r->current_hole - 1;
    if (i < 0) {
        i = 0;
    }
    if (i >= SLAB_HOLES_MAX) {
        i = SLAB_HOLES_MAX - 1;
    }
    return &r->holes[i];
}

static void clamp_strokes(slab_hole_t *h)
{
    if (h->strokes < SLAB_STROKES_MIN) {
        h->strokes = SLAB_STROKES_MIN;
    }
    if (h->strokes > SLAB_STROKES_MAX) {
        h->strokes = SLAB_STROKES_MAX;
    }
    if (h->putts > h->strokes) {
        h->putts = h->strokes;
    }
}

static void clamp_putts(slab_hole_t *h)
{
    /* putts is uint8_t; SLAB_PUTTS_MIN is 0 — never compare unsigned < 0. */
    if (h->putts > SLAB_PUTTS_MAX) {
        h->putts = SLAB_PUTTS_MAX;
    }
    if (h->putts > h->strokes) {
        h->putts = h->strokes;
    }
}

static uint8_t cycle_drive(uint8_t cur_sel, int dir)
{
    /* L ↔ fairway(H) ↔ R. Never NA on a drive hole. */
    static const uint8_t order[] = {SLAB_FWY_L, SLAB_FWY_H, SLAB_FWY_R};
    int idx = 1;
    for (int i = 0; i < 3; i++) {
        if (order[i] == cur_sel) {
            idx = i;
            break;
        }
    }
    idx += dir;
    if (idx < 0) {
        idx = 2;
    }
    if (idx > 2) {
        idx = 0;
    }
    return order[idx];
}

static void lock_and_advance(slab_round_t *r)
{
    slab_hole_t *h = cur(r);
    if (h->par == 3) {
        h->fairway = SLAB_FWY_NA;
    } else {
        h->fairway = r->drive_sel;
        if (h->fairway == SLAB_FWY_NA) {
            h->fairway = SLAB_FWY_H;
        }
    }
    h->locked = 1;
    if (r->current_hole >= r->holes_played) {
        r->ui = SLAB_UI_ROUND_COMPLETE_SYNC;
    } else {
        r->current_hole++;
        slab_hole_t *n = cur(r);
        if (!n->locked) {
            slab_hole_apply_defaults(n);
        }
        r->drive_sel = (n->par == 3) ? SLAB_FWY_NA : SLAB_FWY_H;
        r->ui = SLAB_UI_DEFAULT_HOLE;
    }
}

slab_refresh_t slab_apply_event(slab_round_t *r, slab_evt_t ev)
{
    slab_ui_t prev = (slab_ui_t)r->ui;
    bool value_only = false;
    slab_hole_t *h = cur(r);

    if (ev == SLAB_EVT_NONE) {
        return SLAB_REFRESH_NONE;
    }

    switch (r->ui) {
    case SLAB_UI_DEFAULT_HOLE:
        if (ev == SLAB_EVT_PLUS || ev == SLAB_EVT_MINUS) {
            r->ui = SLAB_UI_STROKE_EDIT;
            if (ev == SLAB_EVT_PLUS) {
                h->strokes++;
            } else if (h->strokes > SLAB_STROKES_MIN) {
                h->strokes--;
            }
            clamp_strokes(h);
        } else if (ev == SLAB_EVT_NEXT) {
            r->ui = SLAB_UI_STROKE_EDIT;
        } else if (ev == SLAB_EVT_BACK || ev == SLAB_EVT_MODE) {
            /* MODE during play is not a GIR prompt. Ignore. */
        }
        break;

    case SLAB_UI_STROKE_EDIT:
        if (ev == SLAB_EVT_PLUS) {
            h->strokes++;
            clamp_strokes(h);
            value_only = true;
        } else if (ev == SLAB_EVT_MINUS) {
            if (h->strokes > SLAB_STROKES_MIN) {
                h->strokes--;
            }
            clamp_strokes(h);
            value_only = true;
        } else if (ev == SLAB_EVT_NEXT) {
            r->ui = SLAB_UI_PUTTS_INPUT;
        } else if (ev == SLAB_EVT_BACK || ev == SLAB_EVT_MODE) {
            r->ui = SLAB_UI_DEFAULT_HOLE;
        }
        /* PUTT is not an event — sequential lock, no skip. */
        break;

    case SLAB_UI_PUTTS_INPUT:
        if (ev == SLAB_EVT_PLUS) {
            h->putts++;
            clamp_putts(h);
            value_only = true;
        } else if (ev == SLAB_EVT_MINUS) {
            if (h->putts > SLAB_PUTTS_MIN) {
                h->putts--;
            }
            clamp_putts(h);
            value_only = true;
        } else if (ev == SLAB_EVT_NEXT) {
            if (h->par == 3) {
                r->drive_sel = SLAB_FWY_NA;
            } else if (r->drive_sel == SLAB_FWY_NA) {
                r->drive_sel = SLAB_FWY_H;
            }
            r->ui = SLAB_UI_END_HOLE_CONFIRM;
        } else if (ev == SLAB_EVT_BACK || ev == SLAB_EVT_MODE) {
            r->ui = SLAB_UI_STROKE_EDIT;
        }
        break;

    case SLAB_UI_END_HOLE_CONFIRM:
        if (h->par != 3 && (ev == SLAB_EVT_PLUS || ev == SLAB_EVT_MODE)) {
            r->drive_sel = cycle_drive(r->drive_sel, +1);
            value_only = true;
        } else if (h->par != 3 && ev == SLAB_EVT_MINUS) {
            r->drive_sel = cycle_drive(r->drive_sel, -1);
            value_only = true;
        } else if (ev == SLAB_EVT_NEXT || ev == SLAB_EVT_LONG_NEXT) {
            lock_and_advance(r);
        } else if (ev == SLAB_EVT_BACK) {
            r->ui = SLAB_UI_PUTTS_INPUT;
        }
        break;

    case SLAB_UI_ROUND_COMPLETE_SYNC:
        if (ev == SLAB_EVT_MODE || ev == SLAB_EVT_NEXT) {
            r->ui = SLAB_UI_DERIVED_STATS;
        }
        break;

    case SLAB_UI_DERIVED_STATS:
        if (ev == SLAB_EVT_MODE || ev == SLAB_EVT_BACK || ev == SLAB_EVT_NEXT) {
            r->ui = SLAB_UI_ROUND_COMPLETE_SYNC;
        }
        break;

    default:
        break;
    }

    slab_sync_ble_gate(r);

    if (r->ui != prev) {
        return SLAB_REFRESH_FULL;
    }
    if (value_only) {
        return SLAB_REFRESH_PARTIAL;
    }
    return SLAB_REFRESH_NONE;
}
