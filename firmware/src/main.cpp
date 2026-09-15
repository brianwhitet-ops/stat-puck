#if !defined(SLAB_HOST)

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
}

using Adafruit_LittleFS_Namespace::File;
using Adafruit_LittleFS_Namespace::FILE_O_READ;
using Adafruit_LittleFS_Namespace::FILE_O_WRITE;

static GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> display(
    GxEPD2_290_T94_V2(SLAB_PIN_EPD_CS, SLAB_PIN_EPD_DC, SLAB_PIN_EPD_RST,
                      SLAB_PIN_EPD_BUSY));

static slab_round_t g_round;
static slab_input_t g_input;
static slab_epd_t g_epd;
static bool g_advertising;

#define NV_PATH "/slab_round.bin"

extern "C" int slab_persist_device_write(const void *data, unsigned len)
{
    File f(InternalFS);
    if (!f.open(NV_PATH, FILE_O_WRITE)) {
        return -1;
    }
    f.seek(0);
    int n = f.write((const uint8_t *)data, len);
    f.close();
    return n == (int)len ? 0 : -1;
}

extern "C" int slab_persist_device_read(void *data, unsigned len)
{
    File f(InternalFS);
    if (!f.open(NV_PATH, FILE_O_READ)) {
        return -1;
    }
    int n = f.read((uint8_t *)data, len);
    f.close();
    return n == (int)len ? 0 : -1;
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
    bool want = slab_ble_should_advertise(&g_round);
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

void setup()
{
    Serial.begin(115200);
    for (int i = 0; i < SLAB_BTN_COUNT; i++) {
        pinMode(kBtnPins[i], INPUT_PULLUP);
    }
    InternalFS.begin();
    display.init(115200, true, 2, false);
    display.setRotation(1);

    slab_input_init(&g_input);
    slab_epd_init(&g_epd);
    if (slab_persist_load(&g_round) != 0) {
        slab_round_init(&g_round);
    }
    slab_sync_ble_gate(&g_round);

    slab_fb_t fb;
    slab_ui_render(&fb, &g_round);
    push_fb(&fb, SLAB_REFRESH_FULL);
    ble_gate_apply();
}

void loop()
{
    uint8_t raw[SLAB_BTN_COUNT];
    for (int i = 0; i < SLAB_BTN_COUNT; i++) {
        raw[i] = digitalRead(kBtnPins[i]) == LOW ? 1 : 0;
    }
    slab_evt_t ev = slab_input_poll(&g_input, raw, 10, 0, 0);
    if (ev != SLAB_EVT_NONE) {
        slab_refresh_t hint = slab_apply_event(&g_round, ev);
        slab_refresh_t mode = slab_epd_choose(&g_epd, hint);
        if (mode != SLAB_REFRESH_NONE) {
            slab_fb_t fb;
            slab_ui_render(&fb, &g_round);
            push_fb(&fb, mode);
            slab_persist_save(&g_round);
            ble_gate_apply();
        }
    }
    delay(10);
}

#endif
