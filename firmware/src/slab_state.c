#include "slab_state.h"

#include <string.h>
#include <stdio.h>

static const uint8_t kDefaultPars[SLAB_HOLES_MAX] = {
    4, 4, 3, 5, 4, 4, 4, 3, 5,
    4, 4, 3, 5, 4, 4, 3, 4, 5
};

/* Hole 7 is locked to the Instinct golden (412 YD). */
static const uint16_t kYards[SLAB_HOLES_MAX] = {
    394, 401, 168, 528, 387, 376,
    412, 172, 541, 398, 405, 155,
    533, 381, 390, 161, 408, 552
};

int slab_course_yards(int hole)
{
    if (hole < 1 || hole > SLAB_HOLES_MAX) {
        return 0;
    }
    return (int)kYards[hole - 1];
}

static bool identifier(const char *s, size_t cap)
{
    if (!s || !s[0]) return false;
    for (size_t i = 0; i < cap; i++) {
        unsigned char c = (unsigned char)s[i];
        if (!c) return true;
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == ':' || c == '_' || c == '-')) return false;
    }
    return false;
}

void slab_hole_apply_defaults(slab_hole_t *h)
{
    h->strokes = 0;
    h->putts = 0;
    h->fairway = SLAB_FWY_NA;
    h->locked = 0;
    h->captured = 0;
}

int slab_round_start(slab_round_t *r, const char *device_id, uint32_t sequence,
                     uint8_t holes, const uint8_t *pars)
{
    if (!r || !identifier(device_id, SLAB_DEVICE_ID_SIZE) || !sequence ||
        (holes != 9 && holes != 18)) return -1;
    if (pars) {
        for (int i = 0; i < holes; i++)
            if (pars[i] != 0 && (pars[i] < 3 || pars[i] > 5)) return -1;
    }
    memset(r, 0, sizeof(*r));
    r->magic = SLAB_PERSIST_MAGIC;
    r->version = SLAB_PERSIST_VERSION;
    r->holes_played = holes;
    r->current_hole = 1;
    r->round_sequence = sequence;
    snprintf(r->device_id, sizeof(r->device_id), "%s", device_id);
    snprintf(r->round_id, sizeof(r->round_id), "%s:%08lx", device_id, (unsigned long)sequence);
    memcpy(r->course_name, "QUICK START", 12);
    for (int i = 0; i < SLAB_HOLES_MAX; i++) {
        r->holes[i].hole = (uint8_t)(i + 1);
        r->holes[i].par = (pars && i < holes) ? pars[i] : 0;
        slab_hole_apply_defaults(&r->holes[i]);
    }
    slab_sync_ble_gate(r);
    return 0;
}

/* Bench helper only. Device runtime calls slab_round_start with real identity. */
void slab_round_init(slab_round_t *r)
{
    (void)slab_round_start(r, "bench", 1, SLAB_HOLES_DEFAULT, NULL);
}

bool slab_round_valid(const slab_round_t *r)
{
    if (!r || r->magic != SLAB_PERSIST_MAGIC || r->version != SLAB_PERSIST_VERSION ||
        (r->holes_played != 9 && r->holes_played != 18) || r->current_hole < 1 ||
        r->current_hole > r->holes_played || r->ui >= SLAB_UI_COUNT ||
        r->drive_sel > SLAB_FWY_R || r->synced > 1 || r->ble_advertise > 1 ||
        !r->round_sequence || !identifier(r->device_id, sizeof(r->device_id)) ||
        !identifier(r->round_id, sizeof(r->round_id)) ||
        !memchr(r->course_name, 0, sizeof(r->course_name)) || !memchr(r->tees, 0, sizeof(r->tees))) return false;
    for (int i = 0; i < r->holes_played; i++) {
        const slab_hole_t *h = &r->holes[i];
        if (h->hole != i + 1 || (h->par != 0 && (h->par < 3 || h->par > 5)) ||
            h->locked > 1 || (h->captured & ~7u) || h->fairway > SLAB_FWY_R) return false;
        if (h->captured & SLAB_CAPTURE_STROKES) {
            if (h->strokes < 1 || h->strokes > SLAB_STROKES_MAX) return false;
        } else if (h->strokes) return false;
        if (h->captured & SLAB_CAPTURE_PUTTS) {
            if (!(h->captured & SLAB_CAPTURE_STROKES) || h->putts > h->strokes || h->putts > SLAB_PUTTS_MAX) return false;
        } else if (h->putts) return false;
        if (h->captured & SLAB_CAPTURE_DRIVE) {
            if (h->par == 3 || h->fairway == SLAB_FWY_NA) return false;
        } else if (h->fairway != SLAB_FWY_NA) return false;
    }
    return true;
}

bool slab_round_complete(const slab_round_t *r)
{
    if (!slab_round_valid(r)) return false;
    if (r->ui != SLAB_UI_ROUND_COMPLETE_SYNC && r->ui != SLAB_UI_DERIVED_STATS) return false;
    for (int i = 0; i < r->holes_played; i++)
        if (!r->holes[i].locked || (r->holes[i].captured & 3u) != 3u) return false;
    return true;
}

bool slab_round_finish_nine(slab_round_t *r)
{
    if (!r || r->holes_played != 18 || r->current_hole != 10 ||
        r->ui != SLAB_UI_DEFAULT_HOLE || r->holes[9].captured) return false;
    for (int i = 0; i < 9; i++)
        if (!r->holes[i].locked || (r->holes[i].captured & 3u) != 3u) return false;
    r->holes_played = 9;
    r->current_hole = 9;
    r->ui = SLAB_UI_ROUND_COMPLETE_SYNC;
    slab_sync_ble_gate(r);
    return true;
}

static void lock_played(slab_round_t *r, int last_inclusive, const uint8_t *strokes,
                        const uint8_t *putts, const uint8_t *fwy)
{
    for (int i = 0; i < last_inclusive; i++) {
        r->holes[i].strokes = strokes[i];
        r->holes[i].putts = putts[i];
        r->holes[i].fairway = (r->holes[i].par == 3) ? SLAB_FWY_NA : fwy[i];
        r->holes[i].captured = SLAB_CAPTURE_STROKES | SLAB_CAPTURE_PUTTS |
            ((r->holes[i].par == 3) ? 0 : SLAB_CAPTURE_DRIVE);
        r->holes[i].locked = 1;
    }
}

void slab_round_init_fixture_hole7(slab_round_t *r)
{
    /* Values encoded in Instinct goldens 01–04: hole 7, strokes 4, putts 2,
     * FAIRWAY, 412 yd, E through 6. */
    (void)slab_round_start(r, "bench", 1, 18, kDefaultPars);
    memcpy(r->tees, "GOLDEN", 7);
    static const uint8_t st[] = {4, 4, 3, 5, 4, 4};
    static const uint8_t pu[] = {2, 2, 2, 2, 1, 1};
    static const uint8_t fw[] = {SLAB_FWY_H, SLAB_FWY_H, SLAB_FWY_NA,
                                 SLAB_FWY_H, SLAB_FWY_H, SLAB_FWY_H};
    lock_played(r, 6, st, pu, fw);
    r->current_hole = 7;
    r->holes[6].strokes = 4;
    r->holes[6].putts = 2;
    r->holes[6].fairway = SLAB_FWY_H;
    r->holes[6].captured = 7;
    r->drive_sel = SLAB_FWY_H;
    r->ui = SLAB_UI_DEFAULT_HOLE;
    slab_sync_ble_gate(r);
}

void slab_round_init_fixture_complete(slab_round_t *r)
{
    /* Goldens 05–06: 74 / +2 / GIR 11/18 61% / 31 putts / 1.72 per hole. */
    (void)slab_round_start(r, "bench", 1, 18, kDefaultPars);
    memcpy(r->tees, "GOLDEN", 7);
    static const uint8_t st[] = {4, 4, 3, 5, 4, 4, 4, 4, 5, 4, 4, 3, 5, 4, 4, 3, 4, 6};
    static const uint8_t pu[] = {2, 2, 2, 2, 1, 1, 2, 2, 2, 1, 1, 2, 2, 1, 2, 2, 2, 2};
    static const uint8_t fw[] = {
        SLAB_FWY_H, SLAB_FWY_H, SLAB_FWY_NA, SLAB_FWY_H, SLAB_FWY_H, SLAB_FWY_H,
        SLAB_FWY_H, SLAB_FWY_NA, SLAB_FWY_H, SLAB_FWY_H, SLAB_FWY_H, SLAB_FWY_NA,
        SLAB_FWY_H, SLAB_FWY_H, SLAB_FWY_H, SLAB_FWY_NA, SLAB_FWY_H, SLAB_FWY_H
    };
    lock_played(r, 18, st, pu, fw);
    r->current_hole = 18;
    r->ui = SLAB_UI_ROUND_COMPLETE_SYNC;
    r->holes_played = 18;
    slab_sync_ble_gate(r);
}

bool slab_hole_gir_known(const slab_hole_t *h)
{
    return h && h->par >= 3 && h->par <= 5 && (h->captured & 3u) == 3u &&
           h->strokes >= 1 && h->putts <= h->strokes;
}

bool slab_hole_gir(const slab_hole_t *h)
{
    if (!slab_hole_gir_known(h)) return false;
    /* GIR derived only: (strokes − putts) ≤ (par − 2). Never prompted. */
    int approach = (int)h->strokes - (int)h->putts;
    int need = (int)h->par - 2;
    return approach <= need;
}

int slab_hole_vs_par(const slab_hole_t *h)
{
    if (!(h->captured & SLAB_CAPTURE_STROKES) || !h->par) return 0;
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
        if (r->holes[i].locked && (r->holes[i].captured & SLAB_CAPTURE_PUTTS)) {
            n += r->holes[i].putts;
        }
    }
    return n;
}

int slab_round_putt_holes(const slab_round_t *r)
{
    int n = 0;
    for (int i = 0; i < r->holes_played; i++)
        if (r->holes[i].locked && (r->holes[i].captured & SLAB_CAPTURE_PUTTS)) n++;
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
        if (r->holes[i].locked && slab_hole_gir_known(&r->holes[i])) {
            n++;
        }
    }
    return n;
}

int slab_round_fwy_hits(const slab_round_t *r)
{
    int n = 0;
    for (int i = 0; i < r->holes_played; i++) {
        if (r->holes[i].locked && r->holes[i].par >= 4 &&
            (r->holes[i].captured & SLAB_CAPTURE_DRIVE) &&
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
        if (r->holes[i].locked && r->holes[i].par >= 4 &&
            (r->holes[i].captured & SLAB_CAPTURE_DRIVE)) {
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
    if (h->strokes > SLAB_STROKES_MAX) {
        h->strokes = SLAB_STROKES_MAX;
    }
    if (h->putts > h->strokes) {
        h->putts = 0;
        h->captured &= (uint8_t)~SLAB_CAPTURE_PUTTS;
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
    /* First deliberate input leaves unset; later inputs cycle L/H/R. */
    if (cur_sel == SLAB_FWY_NA) return dir > 0 ? SLAB_FWY_L : SLAB_FWY_R;
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
        if (h->fairway != SLAB_FWY_NA) h->captured |= SLAB_CAPTURE_DRIVE;
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
        r->drive_sel = n->fairway;
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
        if (ev == SLAB_EVT_LONG_NEXT && slab_round_finish_nine(r)) break;
        if (ev == SLAB_EVT_PLUS || ev == SLAB_EVT_MINUS) {
            r->ui = SLAB_UI_STROKE_EDIT;
            if (ev == SLAB_EVT_PLUS) {
                h->strokes++;
            } else if (h->strokes > SLAB_STROKES_MIN) {
                h->strokes--;
            }
            clamp_strokes(h);
            if (h->strokes) h->captured |= SLAB_CAPTURE_STROKES;
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
            if (h->strokes) h->captured |= SLAB_CAPTURE_STROKES;
            value_only = true;
        } else if (ev == SLAB_EVT_MINUS) {
            if (h->strokes > SLAB_STROKES_MIN) {
                h->strokes--;
            }
            clamp_strokes(h);
            if (h->strokes) h->captured |= SLAB_CAPTURE_STROKES;
            value_only = true;
        } else if (ev == SLAB_EVT_NEXT && (h->captured & SLAB_CAPTURE_STROKES)) {
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
            if (h->captured & SLAB_CAPTURE_STROKES) h->captured |= SLAB_CAPTURE_PUTTS;
            value_only = true;
        } else if (ev == SLAB_EVT_MINUS) {
            if (h->putts > SLAB_PUTTS_MIN) {
                h->putts--;
            }
            clamp_putts(h);
            if (h->captured & SLAB_CAPTURE_STROKES) h->captured |= SLAB_CAPTURE_PUTTS;
            value_only = true;
        } else if (ev == SLAB_EVT_NEXT && (h->captured & 3u) == 3u) {
            r->drive_sel = h->fairway;
            r->ui = SLAB_UI_END_HOLE_CONFIRM;
        } else if (ev == SLAB_EVT_BACK || ev == SLAB_EVT_MODE) {
            r->ui = SLAB_UI_STROKE_EDIT;
        }
        break;

    case SLAB_UI_END_HOLE_CONFIRM:
        if (h->par != 3 && (ev == SLAB_EVT_PLUS || ev == SLAB_EVT_MODE)) {
            r->drive_sel = cycle_drive(r->drive_sel, +1);
            h->fairway = r->drive_sel;
            h->captured |= SLAB_CAPTURE_DRIVE;
            value_only = true;
        } else if (h->par != 3 && ev == SLAB_EVT_MINUS) {
            r->drive_sel = cycle_drive(r->drive_sel, -1);
            h->fairway = r->drive_sel;
            h->captured |= SLAB_CAPTURE_DRIVE;
            value_only = true;
        } else if ((ev == SLAB_EVT_NEXT || ev == SLAB_EVT_LONG_NEXT) && (h->captured & 3u) == 3u) {
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
