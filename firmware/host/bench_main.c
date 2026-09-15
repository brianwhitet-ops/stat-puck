#include "slab_fb.h"
#include "slab_state.h"
#include "slab_ui.h"
#include "slab_ble.h"
#include "slab_persist.h"
#include "slab_input.h"
#include "slab_epd.h"
#include "png_write.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static int ensure_dir(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0) {
        return 0;
    }
    return mkdir(path, 0755);
}

static int render_named(const slab_round_t *r, const char *dir, const char *name)
{
    slab_fb_t fb;
    slab_ui_render(&fb, r);
    char path[256];
    snprintf(path, sizeof(path), "%s/%s.png", dir, name);
    if (slab_write_png(path, &fb) != 0) {
        fprintf(stderr, "write failed: %s\n", path);
        return -1;
    }
    printf("wrote %s  %s  ble=%d  %dx%d\n", path, slab_ui_name((slab_ui_t)r->ui),
           r->ble_advertise, SLAB_FB_W, SLAB_FB_H);
    return 0;
}

int main(int argc, char **argv)
{
    const char *out = "../testdata/out";
    if (argc >= 2) {
        out = argv[1];
    }
    ensure_dir(out);

    slab_round_t r;
    int rc = 0;

    slab_round_init_fixture_hole7(&r);
    r.ui = SLAB_UI_DEFAULT_HOLE;
    slab_sync_ble_gate(&r);
    rc |= render_named(&r, out, "01-default-hole");

    r.ui = SLAB_UI_STROKE_EDIT;
    slab_sync_ble_gate(&r);
    rc |= render_named(&r, out, "02-stroke-edit");

    r.ui = SLAB_UI_PUTTS_INPUT;
    slab_sync_ble_gate(&r);
    rc |= render_named(&r, out, "03-putts-input");

    r.ui = SLAB_UI_END_HOLE_CONFIRM;
    r.drive_sel = SLAB_FWY_H;
    slab_sync_ble_gate(&r);
    rc |= render_named(&r, out, "04-end-hole-confirm");

    slab_round_init_fixture_complete(&r);
    r.ui = SLAB_UI_ROUND_COMPLETE_SYNC;
    slab_sync_ble_gate(&r);
    rc |= render_named(&r, out, "05-round-complete-sync");

    r.ui = SLAB_UI_DERIVED_STATS;
    slab_sync_ble_gate(&r);
    rc |= render_named(&r, out, "06-derived-stats");

    return rc ? 1 : 0;
}
