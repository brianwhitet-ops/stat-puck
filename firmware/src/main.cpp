#if !SLAB_HOST

#if defined(SLAB_FACTORY_PROVISION)
#error "Refusing to build scoring firmware with SLAB_FACTORY_PROVISION"
#endif

#include <Arduino.h>
#include <SPI.h>
#include <GxEPD2_BW.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

extern "C" {
#include "slab_state.h"
#include "slab_ui.h"
#include "slab_input.h"
#include "slab_persist.h"
#include "slab_ble.h"
#include "slab_epd.h"
#include "slab_board.h"
#include "slab_font.h"
#include "slab_store.h"
}

using Adafruit_LittleFS_Namespace::File;
using Adafruit_LittleFS_Namespace::FILE_O_READ;
using Adafruit_LittleFS_Namespace::FILE_O_WRITE;

static GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> display(
    GxEPD2_290_T94_V2(SLAB_PIN_EPD_CS, SLAB_PIN_EPD_DC, SLAB_PIN_EPD_RST,
                      SLAB_PIN_EPD_BUSY));

static slab_store_t g_store;
static slab_store_t g_candidate;
static slab_fb_t g_fb;
static slab_round_t g_round;
static slab_input_t g_input;
static slab_epd_t g_epd;
static bool g_advertising;
static bool g_start_screen = true;
static bool g_storage_error;
static uint8_t g_selected_holes = 18;
static char g_device_id[SLAB_DEVICE_ID_SIZE];
static const char *kSlotPaths[] = {"/slab_store.0", "/slab_store.1"};

extern "C" int slab_persist_slot_write(unsigned slot, const void *data, unsigned len)
{
    if (slot > 1) return -1;
    // This is always the inactive journal slot. The committed slot is retained.
    if (InternalFS.exists(kSlotPaths[slot]) && !InternalFS.remove(kSlotPaths[slot])) return -1;
    File f(InternalFS);
    if (!f.open(kSlotPaths[slot], FILE_O_WRITE)) return -1;
    int n = f.write((const uint8_t *)data, len);
    f.flush();
    f.close();
    return n == (int)len ? 0 : -1;
}
extern "C" int slab_persist_slot_read(unsigned slot, void *data, unsigned len)
{
    if (slot > 1) return -1;
    if (!InternalFS.exists(kSlotPaths[slot])) {
        // Legacy data cannot tell entered scores from fabricated defaults.
        // Preserve it for explicit service migration; never erase or reset it.
        return InternalFS.exists("/slab_round.bin") ? SLAB_PERSIST_ERROR : SLAB_PERSIST_EMPTY;
    }
    File f(InternalFS);
    if (!f.open(kSlotPaths[slot], FILE_O_READ)) return -1;
    bool size_ok = f.size() == len;
    int n = f.read((uint8_t *)data, len);
    f.close();
    return size_ok && n == (int)len ? 0 : -1;
}

static void push_fb(const slab_fb_t *fb, slab_refresh_t mode)
{
    display.setRotation(1); /* 296 x 128 landscape */
    if (mode == SLAB_REFRESH_PARTIAL) {
        display.setPartialWindow(0, 0, SLAB_FB_W, SLAB_FB_H);
    } else {
        display.setFullWindow();
    }
    display.firstPage();
    do {
        for (int y = 0; y < SLAB_FB_H; y++) {
            for (int x = 0; x < SLAB_FB_W; x++) {
                display.drawPixel(x, y, slab_fb_get(fb, x, y) ? GxEPD_BLACK : GxEPD_WHITE);
            }
        }
    } while (display.nextPage());
    display.hibernate();
}

static void ble_gate_apply(void)
{
    bool want = !g_storage_error && !g_start_screen && !g_store.active_present &&
        g_store.count && slab_ble_should_advertise(&g_round);
    if (want == g_advertising) {
        return;
    }
    g_advertising = want;
    /*
     * Stub: Bluefruit advertising is started only here.
     * Mid-round states must never reach startAdvertising().
     */
    Serial.print("BLE advertise ");
    Serial.println(want ? "ON (ROUND_COMPLETE_SYNC)" : "OFF");
}

static const uint8_t kBtnPins[SLAB_BTN_COUNT] = {
    SLAB_PIN_BTN_PLUS, SLAB_PIN_BTN_MINUS, SLAB_PIN_BTN_PUTT, SLAB_PIN_BTN_MODE,
    SLAB_PIN_BTN_NEXT};

static void render_screen()
{
    slab_fb_t &fb = g_fb;
    if (g_storage_error || g_start_screen) {
        slab_fb_clear(&fb, 0);
        slab_text_m(&fb, 12, 12, g_storage_error ? "STORAGE ERROR" : "QUICK START", 1);
        if (g_storage_error) {
            slab_text_s(&fb, 12, 44, "NO DATA ERASED", 1);
            slab_text_s(&fb, 12, 65, "RESTART / SERVICE", 1);
        } else {
            char line[32];
            snprintf(line, sizeof(line), "%u HOLES", g_selected_holes);
            slab_text_m(&fb, 12, 42, line, 1);
            slab_text_s(&fb, 12, 68, "+/- 9 OR 18 / NEXT START", 1);
            snprintf(line, sizeof(line), "%u SAVED / MODE TO VIEW", g_store.count);
            slab_text_s(&fb, 12, 94, line, 1);
        }
    } else {
        slab_ui_render(&fb, &g_round);
        if (!g_store.active_present && g_round.ui == SLAB_UI_ROUND_COMPLETE_SYNC) {
            slab_fb_fill_rect(&fb, 6, 111, 284, 16, 0);
            slab_text_s(&fb, 8, 113, "SAVED / HOLD NEXT FOR NEW ROUND", 1);
        }
    }
    push_fb(&fb, SLAB_REFRESH_FULL);
    ble_gate_apply();
}

static bool commit_store(const slab_store_t &candidate)
{
    if (slab_persist_save_store(&candidate)) {
        g_storage_error = true;
        render_screen();
        return false;
    }
    g_store = candidate;
    return true;
}

void setup()
{
    Serial.begin(115200);
    snprintf(g_device_id, sizeof(g_device_id), "slab-%08lx%08lx",
             (unsigned long)NRF_FICR->DEVICEID[1], (unsigned long)NRF_FICR->DEVICEID[0]);
    for (int i = 0; i < SLAB_BTN_COUNT; i++) pinMode(kBtnPins[i], INPUT_PULLUP);
    // InternalFileSystem::begin auto-formats on mount failure. Use the base
    // mount-only operation so a damaged filesystem is never silently erased.
    // Fresh devices must be formatted by the factory/service provisioning step.
    bool fs_ok = InternalFS.Adafruit_LittleFS::begin();
    display.init(115200, true, 2, false);
    display.setRotation(1);
    slab_input_init(&g_input);
    slab_epd_init(&g_epd);
    int status = fs_ok ? slab_persist_load_store(&g_store) : SLAB_PERSIST_ERROR;
    if (status == SLAB_PERSIST_EMPTY) {
        slab_store_init(&g_store);
        g_storage_error = slab_persist_save_store(&g_store) != 0;
    } else g_storage_error = status != 0;
    if (!g_storage_error && g_store.active_present) {
        g_round = g_store.active;
        g_start_screen = false;
    } else if (!g_storage_error && g_store.count) {
        g_round = g_store.queued[g_store.count - 1];
        g_start_screen = false;
    }
    render_screen();
}

void loop()
{
    if (g_storage_error) { delay(10); return; }
    uint8_t raw[SLAB_BTN_COUNT];
    for (int i = 0; i < SLAB_BTN_COUNT; i++) raw[i] = digitalRead(kBtnPins[i]) == LOW ? 1 : 0;
    slab_evt_t ev = slab_input_poll(&g_input, raw, 10, 0, 0);
    if (ev == SLAB_EVT_NONE) { delay(10); return; }
    if (g_start_screen) {
        if (ev == SLAB_EVT_PLUS || ev == SLAB_EVT_MINUS) g_selected_holes = g_selected_holes == 9 ? 18 : 9;
        else if (ev == SLAB_EVT_NEXT) {
            g_candidate = g_store;
            slab_store_t &candidate = g_candidate;
            int result = slab_store_begin(&candidate, g_device_id, g_selected_holes, NULL);
            if (result == SLAB_STORE_FULL) {
                g_round = g_store.queued[0];
                g_start_screen = false;
            } else if (result || !commit_store(candidate)) {
                g_storage_error = true;
            } else {
                g_round = g_store.active;
                g_start_screen = false;
            }
        } else if (ev == SLAB_EVT_BACK && g_store.count) {
            g_round = g_store.queued[g_store.count - 1];
            g_start_screen = false;
        }
        render_screen();
    } else if (!g_store.active_present) {
        if (ev == SLAB_EVT_LONG_NEXT) g_start_screen = true;
        else (void)slab_apply_event(&g_round, ev);
        render_screen();
    } else {
        g_candidate = g_store;
        slab_store_t &candidate = g_candidate;
        slab_refresh_t hint = slab_apply_event(&candidate.active, ev);
        if (hint != SLAB_REFRESH_NONE) {
            slab_round_t next_view = candidate.active;
            if (slab_round_complete(&candidate.active) && slab_store_finish(&candidate)) {
                g_storage_error = true;
                render_screen();
                return;
            }
            // Commit before displaying a saved state or allowing sync.
            if (!commit_store(candidate)) return;
            g_round = next_view;
            slab_fb_t &fb = g_fb;
            slab_ui_render(&fb, &g_round);
            if (!g_store.active_present) render_screen();
            else {
                push_fb(&fb, slab_epd_choose(&g_epd, hint));
                ble_gate_apply();
            }
        }
    }
    delay(10);
}

#endif
