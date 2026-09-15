#include "slab_ui.h"

#include "slab_font.h"
#include "slab_state.h"

#define INK 1

#define M_X 10
#define RULE_X0 10
#define RULE_X1 285
#define Y_HEAD 5
#define Y_RULE1 24
#define Y_STROKE 30
#define Y_PUTT 54
#define Y_RULE2 78
#define Y_FLAGS 84
#define Y_RULE3 106
#define Y_FOOT 110

static void rule(slab_fb_t *fb, int y)
{
    slab_fb_hline(fb, RULE_X0, RULE_X1, y, INK);
}

static void fmt_i(char *b, int n)
{
    if (n < 0) {
        b[0] = '-';
        n = -n;
        if (n >= 10) {
            b[1] = (char)('0' + (n / 10));
            b[2] = (char)('0' + (n % 10));
            b[3] = 0;
        } else {
            b[1] = (char)('0' + n);
            b[2] = 0;
        }
        return;
    }
    if (n >= 10) {
        b[0] = (char)('0' + (n / 10));
        b[1] = (char)('0' + (n % 10));
        b[2] = 0;
    } else {
        b[0] = (char)('0' + n);
        b[1] = 0;
    }
}

static void fmt_vs(char *b, int v)
{
    if (v == 0) {
        b[0] = 'E';
        b[1] = 0;
        return;
    }
    if (v > 0) {
        b[0] = '+';
        fmt_i(b + 1, v);
        return;
    }
    fmt_i(b, v);
}

static const char *tag_for(slab_ui_t ui)
{
    switch (ui) {
    case SLAB_UI_DEFAULT_HOLE:
        return "";
    case SLAB_UI_STROKE_EDIT:
        return "STROKE";
    case SLAB_UI_PUTTS_INPUT:
        return "PUTTS";
    case SLAB_UI_END_HOLE_CONFIRM:
        return "LOCK";
    case SLAB_UI_ROUND_COMPLETE_SYNC:
        return "SYNC";
    case SLAB_UI_DERIVED_STATS:
        return "STATS";
    default:
        return "";
    }
}

static void draw_value_box(slab_fb_t *fb, int x, int y, const char *val, int selected)
{
    int tw = slab_text_m_width(val);
    int pad_x = 3;
    int pad_y = 2;
    int bw = tw + pad_x * 2;
    int bh = SLAB_FONT_M_H + pad_y * 2;
    slab_text_m(fb, x + pad_x, y, val, INK);
    if (selected) {
        slab_fb_invert_rect(fb, x, y - pad_y, bw, bh);
    }
}

static void draw_chip(slab_fb_t *fb, int x, int y, const char *label, int selected)
{
    int tw = slab_text_s_width(label);
    int pad_x = 4;
    int pad_y = 2;
    int bw = tw + pad_x * 2;
    int bh = SLAB_FONT_S_H + pad_y * 2;
    slab_text_s(fb, x + pad_x, y, label, INK);
    if (selected) {
        slab_fb_invert_rect(fb, x, y - pad_y, bw, bh);
    } else {
        slab_fb_rect(fb, x, y - pad_y, bw, bh, INK);
    }
}

static void header_hole(slab_fb_t *fb, const slab_round_t *r, const slab_hole_t *h)
{
    char n[4];
    char p[4];
    fmt_i(n, h->hole);
    fmt_i(p, h->par);
    slab_text_s(fb, M_X, Y_HEAD, "HOLE", INK);
    slab_text_m(fb, M_X + 40, Y_HEAD - 1, n, INK);

    int par_w = slab_text_s_width("PAR");
    int pv_w = slab_text_m_width(p);
    int right = RULE_X1;
    slab_text_m(fb, right - pv_w, Y_HEAD - 2, p, INK);
    slab_text_s(fb, right - pv_w - 6 - par_w, Y_HEAD, "PAR", INK);

    const char *tag = tag_for((slab_ui_t)r->ui);
    if (tag[0]) {
        int tw = slab_text_s_width(tag);
        slab_text_s(fb, 148 - tw / 2, Y_HEAD + 1, tag, INK);
    }
}

static void footer_vs(slab_fb_t *fb, const slab_round_t *r)
{
    char vs[8];
    char line[24];
    const slab_hole_t *h = &r->holes[r->current_hole - 1];
    int v = slab_hole_vs_par(h);
    fmt_vs(vs, v);
    /* " +1 VS PAR " */
    line[0] = 0;
    int i = 0;
    const char *a = vs;
    while (*a && i < 8) {
        line[i++] = *a++;
    }
    line[i++] = ' ';
    line[i++] = 'V';
    line[i++] = 'S';
    line[i++] = ' ';
    line[i++] = 'P';
    line[i++] = 'A';
    line[i++] = 'R';
    line[i] = 0;
    int w = slab_text_s_width(line);
    slab_text_s(fb, 148 - w / 2, Y_FOOT, line, INK);
}

static void draw_hole_card(slab_fb_t *fb, const slab_round_t *r)
{
    const slab_hole_t *h = &r->holes[r->current_hole - 1];
    char st[4];
    char pu[4];
    fmt_i(st, h->strokes);
    fmt_i(pu, h->putts);

    header_hole(fb, r, h);
    rule(fb, Y_RULE1);

    slab_text_s(fb, M_X, Y_STROKE + 3, "STROKES", INK);
    draw_value_box(fb, 92, Y_STROKE, st, r->ui == SLAB_UI_STROKE_EDIT);

    slab_text_s(fb, M_X, Y_PUTT + 3, "PUTTS", INK);
    draw_value_box(fb, 92, Y_PUTT, pu, r->ui == SLAB_UI_PUTTS_INPUT);

    rule(fb, Y_RULE2);

    if (r->ui == SLAB_UI_END_HOLE_CONFIRM) {
        if (h->par == 3) {
            slab_text_s(fb, M_X, Y_FLAGS + 2, "DRIVE", INK);
            draw_chip(fb, 56, Y_FLAGS + 2, "N/A", 1);
            slab_text_s(fb, 104, Y_FLAGS + 2, "PAR 3", INK);
        } else {
            slab_text_s(fb, M_X, Y_FLAGS + 2, "DRIVE", INK);
            int sel = r->drive_sel;
            draw_chip(fb, 58, Y_FLAGS + 2, "L", sel == SLAB_FWY_L);
            draw_chip(fb, 90, Y_FLAGS + 2, "FWY", sel == SLAB_FWY_H);
            draw_chip(fb, 140, Y_FLAGS + 2, "R", sel == SLAB_FWY_R);
        }
    } else {
        char fc[2] = {slab_fwy_char(h->fairway), 0};
        if (h->par == 3) {
            fc[0] = 'N';
        }
        slab_text_s(fb, M_X, Y_FLAGS + 2, "FWY", INK);
        slab_text_s(fb, 40, Y_FLAGS + 2, fc, INK);
    }

    /* GIR is overlay-only, derived, never a focus. */
    char gc[2] = {slab_hole_gir(h) ? 'Y' : 'N', 0};
    int gir_w = slab_text_s_width("GIR");
    slab_text_s(fb, RULE_X1 - gir_w - 16, Y_FLAGS + 2, "GIR", INK);
    slab_text_s(fb, RULE_X1 - 10, Y_FLAGS + 2, gc, INK);

    rule(fb, Y_RULE3);

    if (r->ui == SLAB_UI_END_HOLE_CONFIRM) {
        char n[4];
        fmt_i(n, h->hole);
        char msg[28];
        /* LOCK HOLE n ? */
        int k = 0;
        const char *p = "LOCK HOLE ";
        while (*p) {
            msg[k++] = *p++;
        }
        p = n;
        while (*p) {
            msg[k++] = *p++;
        }
        msg[k] = 0;
        int w = slab_text_s_width(msg);
        slab_text_s(fb, 148 - w / 2, Y_FOOT, msg, INK);
    } else {
        footer_vs(fb, r);
    }
}

static void draw_sync(slab_fb_t *fb, const slab_round_t *r)
{
    slab_text_s(fb, M_X, Y_HEAD, "ROUND COMPLETE", INK);
    int tw = slab_text_s_width("SYNC");
    slab_text_s(fb, RULE_X1 - tw, Y_HEAD, "SYNC", INK);
    rule(fb, Y_RULE1);

    char left[8];
    char right[8];
    fmt_i(left, r->holes_played);
    fmt_i(right, r->holes_played);
    slab_text_m(fb, M_X, 36, left, INK);
    slab_text_s(fb, M_X + 28, 40, "/", INK);
    slab_text_m(fb, M_X + 40, 36, right, INK);
    slab_text_s(fb, M_X + 70, 40, "HOLES", INK);

    char vs[8];
    fmt_vs(vs, slab_round_vs_par(r, false));
    int vw = slab_text_m_width(vs);
    slab_text_m(fb, RULE_X1 - vw, 36, vs, INK);

    rule(fb, Y_RULE2);
    slab_text_s(fb, M_X, Y_FLAGS + 2, "BLE ADVERTISE", INK);
    draw_chip(fb, 130, Y_FLAGS + 2, "ON", 1);
    rule(fb, Y_RULE3);
    slab_text_s(fb, M_X, Y_FOOT, "PULL CARD", INK);
    slab_text_s(fb, 148, Y_FOOT, "NO MID-ROUND", INK);
}

static void draw_stats(slab_fb_t *fb, const slab_round_t *r)
{
    slab_text_s(fb, M_X, Y_HEAD, "DERIVED", INK);
    int tw = slab_text_s_width("STATS");
    slab_text_s(fb, RULE_X1 - tw, Y_HEAD, "STATS", INK);
    rule(fb, Y_RULE1);

    char a[8], b[8];
    fmt_i(a, slab_round_gir_hits(r));
    fmt_i(b, slab_round_gir_holes(r));
    slab_text_s(fb, M_X, 32, "GIR", INK);
    slab_text_m(fb, 70, 28, a, INK);
    slab_text_s(fb, 100, 32, "/", INK);
    slab_text_m(fb, 112, 28, b, INK);

    fmt_i(a, slab_round_fwy_hits(r));
    fmt_i(b, slab_round_fwy_holes(r));
    slab_text_s(fb, M_X, 54, "FWY", INK);
    slab_text_m(fb, 70, 50, a, INK);
    slab_text_s(fb, 100, 54, "/", INK);
    slab_text_m(fb, 112, 50, b, INK);

    rule(fb, Y_RULE2);
    fmt_i(a, slab_round_putts(r));
    slab_text_s(fb, M_X, Y_FLAGS + 2, "PUTTS", INK);
    slab_text_m(fb, 70, Y_FLAGS - 2, a, INK);
    rule(fb, Y_RULE3);
    slab_text_s(fb, M_X, Y_FOOT, "GIR FROM SCORE-PUTTS VS PAR", INK);
}

void slab_ui_render(slab_fb_t *fb, const slab_round_t *r)
{
    slab_fb_clear(fb, 0);
    slab_fb_rect(fb, 0, 0, SLAB_FB_W, SLAB_FB_H, INK);

    switch (r->ui) {
    case SLAB_UI_ROUND_COMPLETE_SYNC:
        draw_sync(fb, r);
        break;
    case SLAB_UI_DERIVED_STATS:
        draw_stats(fb, r);
        break;
    default:
        draw_hole_card(fb, r);
        break;
    }
}
