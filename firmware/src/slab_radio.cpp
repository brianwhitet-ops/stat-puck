#if !SLAB_HOST

#include <Arduino.h>
#include <bluefruit.h>
#include <string.h>

extern "C" {
#include "slab_radio.h"
#include "slab_ble.h"
#include "slab_store.h"
#include "slab_state.h"
}

/*
 * Bench GATT for nRF Connect. Advertising name is "Slab".
 *
 * Service 534c4201-0000-4e52-8000-00805f9b34fb
 *   534c4202 meta     read  UTF-8 JSON (ids, checksum, byte_length, page_length)
 *   534c4203          reserved; not registered (S140 value cap is 512 bytes)
 *   534c4204 chunk    write offset, read that window of the body
 *   534c4205 ack      write the meta JSON to retire the oldest round
 *   534c4206 result   read  idle / ok / conflict / not_found / invalid / persist_error
 *   534c4210+ page n  read  body slice n * page_length, empty when past the end
 *
 * TODO: this is a read-path MVP plus a single ACK write. Still open: pairing
 * before a destructive ACK, an explicit transfer-begin session, gap/replay
 * checks beyond the chunk window, and a result that waits on the phone's
 * durable commit instead of the device journal alone.
 */

static const char *kUuidService = "534c4201-0000-4e52-8000-00805f9b34fb";
static const char *kUuidMeta = "534c4202-0000-4e52-8000-00805f9b34fb";
static const char *kUuidChunk = "534c4204-0000-4e52-8000-00805f9b34fb";
static const char *kUuidAck = "534c4205-0000-4e52-8000-00805f9b34fb";
static const char *kUuidResult = "534c4206-0000-4e52-8000-00805f9b34fb";

static BLEService g_service(kUuidService);
static BLECharacteristic g_meta(kUuidMeta);
static BLECharacteristic g_chunk(kUuidChunk);
static BLECharacteristic g_ack(kUuidAck);
static BLECharacteristic g_result(kUuidResult);
static BLECharacteristic g_pages[SLAB_BLE_PAGE_COUNT];

static char g_json_buf[SLAB_PAYLOAD_SIZE];
static int g_json_len;
/* S140 caps one attribute at BLE_GATTS_VAR_ATTR_LEN_MAX (512). A full round
 * is larger than that, so the body is the page characteristics. long_read
 * stays 0 until a stack can hold the whole object in one value. */
static const int g_long_ok = 0;
static bool g_radio_up;
static bool g_want_adv;
static uint8_t g_page_mem[SLAB_BLE_PAGE_COUNT][SLAB_BLE_PAGE_LEN];
static uint8_t g_window[SLAB_BLE_CHUNK_LEN];
static char g_ack_buf[256];
static volatile bool g_ack_pending;
static char g_page_uuid[SLAB_BLE_PAGE_COUNT][40];
static char g_page_label[SLAB_BLE_PAGE_COUNT][24];

static void stop_adv(void)
{
    if (!g_radio_up) return;
    Bluefruit.Advertising.stop();
    if (Bluefruit.connected()) {
        uint16_t conn = Bluefruit.connHandle();
        if (conn != BLE_CONN_HANDLE_INVALID) Bluefruit.disconnect(conn);
    }
}

static void start_adv(void)
{
    if (!g_radio_up || Bluefruit.Advertising.isRunning()) return;
    Bluefruit.Advertising.start(0);
}

static void disconnect_cb(uint16_t conn_handle, uint8_t reason)
{
    (void)conn_handle;
    (void)reason;
    if (g_want_adv) Bluefruit.Advertising.start(0);
}

static void connect_cb(uint16_t conn_handle)
{
    BLEConnection *conn = Bluefruit.Connection(conn_handle);
    if (!conn) return;
    conn->requestMtuExchange(247);
    conn->requestDataLengthUpdate();
}

static void refresh_window(uint32_t offset)
{
    uint16_t n = 0;
    if (g_json_len > 0 && offset < (uint32_t)g_json_len) {
        uint32_t remain = (uint32_t)g_json_len - offset;
        if (remain > SLAB_BLE_CHUNK_LEN) remain = SLAB_BLE_CHUNK_LEN;
        n = (uint16_t)remain;
        memcpy(g_window, g_json_buf + offset, n);
    }
    g_chunk.write(g_window, n);
}

static bool parse_offset(const uint8_t *data, uint16_t len, uint32_t *offset)
{
    if (!data || !offset || !len) return false;
    if (len == 4) {
        *offset = (uint32_t)data[0] | ((uint32_t)data[1] << 8) |
                  ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
        return true;
    }
    if (len == 2) {
        *offset = (uint32_t)data[0] | ((uint32_t)data[1] << 8);
        return true;
    }
    if (len >= 8) return false;
    uint32_t v = 0;
    for (uint16_t i = 0; i < len; i++) {
        if (data[i] < '0' || data[i] > '9') return false;
        v = v * 10u + (uint32_t)(data[i] - '0');
    }
    *offset = v;
    return true;
}

static void on_chunk(uint16_t conn_handle, BLECharacteristic *chr, uint8_t *data, uint16_t len)
{
    (void)conn_handle;
    (void)chr;
    uint32_t offset = 0;
    if (!parse_offset(data, len, &offset)) return;
    refresh_window(offset);
}

static void on_ack(uint16_t conn_handle, BLECharacteristic *chr, uint8_t *data, uint16_t len)
{
    (void)conn_handle;
    (void)chr;
    if (g_ack_pending || !data || !len) return;
    if (len >= sizeof(g_ack_buf)) len = sizeof(g_ack_buf) - 1;
    memcpy(g_ack_buf, data, len);
    g_ack_buf[len] = 0;
    g_ack_pending = true;
}

static void publish_body(void)
{
    for (int i = 0; i < SLAB_BLE_PAGE_COUNT; i++) {
        int off = i * SLAB_BLE_PAGE_LEN;
        int n = 0;
        if (off < g_json_len) {
            n = g_json_len - off;
            if (n > SLAB_BLE_PAGE_LEN) n = SLAB_BLE_PAGE_LEN;
            g_pages[i].write(g_json_buf + off, (uint16_t)n);
        } else {
            g_pages[i].write(g_page_mem[i], 0);
        }
    }
    refresh_window(0);
}

extern "C" void slab_radio_init(void)
{
    Bluefruit.autoConnLed(false);
    Bluefruit.configAttrTableSize(0x2000);
    Bluefruit.begin();
    Bluefruit.setTxPower(4);
    Bluefruit.setName("Slab");
    Bluefruit.Periph.setConnectCallback(connect_cb);
    Bluefruit.Periph.setDisconnectCallback(disconnect_cb);

    g_service.begin();

    g_meta.setProperties(CHR_PROPS_READ);
    g_meta.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
    g_meta.setMaxLen(280);
    g_meta.setUserDescriptor("Slab round meta");
    g_meta.begin();

    g_chunk.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
    g_chunk.setPermission(SECMODE_OPEN, SECMODE_OPEN);
    g_chunk.setMaxLen(SLAB_BLE_CHUNK_LEN);
    g_chunk.setUserDescriptor("Slab chunk window");
    g_chunk.setWriteCallback(on_chunk);
    g_chunk.begin();

    g_ack.setProperties(CHR_PROPS_WRITE);
    g_ack.setPermission(SECMODE_OPEN, SECMODE_OPEN);
    g_ack.setMaxLen(sizeof(g_ack_buf) - 1);
    g_ack.setUserDescriptor("Slab round ACK");
    g_ack.setWriteCallback(on_ack);
    g_ack.begin();

    g_result.setProperties(CHR_PROPS_READ);
    g_result.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
    g_result.setMaxLen(16);
    g_result.setUserDescriptor("Slab ACK result");
    g_result.begin();
    g_result.write("idle");

    for (int i = 0; i < SLAB_BLE_PAGE_COUNT; i++) {
        snprintf(g_page_uuid[i], sizeof(g_page_uuid[i]),
                 "534c42%02x-0000-4e52-8000-00805f9b34fb", 0x10 + i);
        snprintf(g_page_label[i], sizeof(g_page_label[i]), "Slab JSON page %d", i);
        g_pages[i].setUuid(g_page_uuid[i]);
        g_pages[i].setProperties(CHR_PROPS_READ);
        g_pages[i].setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
        g_pages[i].setBuffer(g_page_mem[i], SLAB_BLE_PAGE_LEN);
        g_pages[i].setUserDescriptor(g_page_label[i]);
        if (g_pages[i].begin() != ERROR_NONE) {
            Serial.print("BLE page failed ");
            Serial.println(i);
        }
    }

    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addService(g_service);
    Bluefruit.Advertising.addName();
    Bluefruit.Advertising.setInterval(32, 244);
    Bluefruit.Advertising.setFastTimeout(30);
    g_radio_up = true;
    Serial.println("BLE GATT ready, name Slab, advertising off");
}

extern "C" void slab_radio_set_status(const char *status)
{
    if (!g_radio_up || !status) return;
    g_result.write(status);
}

extern "C" int slab_radio_consume_ack(slab_store_t *candidate)
{
    if (!g_ack_pending) return 1;
    char device[SLAB_DEVICE_ID_SIZE];
    char round_id[SLAB_ROUND_ID_SIZE];
    char checksum[15];
    uint32_t sequence = 0;
    bool parsed = slab_ble_parse_ack(g_ack_buf, device, sizeof(device), round_id, sizeof(round_id),
                                     &sequence, checksum);
    g_ack_pending = false;
    if (!parsed || !candidate) return SLAB_STORE_INVALID;
    return slab_store_ack(candidate, device, round_id, sequence, checksum);
}

extern "C" void slab_radio_apply(const slab_store_t *store, bool advertise)
{
    if (!g_radio_up) return;
    g_json_len = 0;
    g_json_buf[0] = 0;
    bool serve = advertise && store && store->count && !store->active_present &&
                 slab_round_complete(&store->queued[0]) &&
                 slab_ble_should_advertise(&store->queued[0]);
    if (serve) {
        int n = slab_ble_build_payload(&store->queued[0], g_json_buf, sizeof(g_json_buf));
        if (n <= 0) serve = false;
        else g_json_len = n;
    }
    if (!serve) {
        g_json_buf[0] = 0;
        g_json_len = 0;
        g_meta.write("{\"ready\":false}");
    } else {
        char meta[280];
        int m = slab_ble_build_meta(&store->queued[0], g_json_len, SLAB_BLE_PAGE_LEN, g_long_ok ? 1 : 0,
                                   meta, sizeof(meta));
        if (m > 0) g_meta.write(meta);
        else g_meta.write("{\"ready\":false}");
    }
    publish_body();
    g_want_adv = serve;
    if (serve) {
        start_adv();
        Serial.print("BLE advertise ON bytes=");
        Serial.println(g_json_len);
    } else {
        stop_adv();
        Serial.println("BLE advertise OFF");
    }
}

extern "C" void slab_radio_sleep(void)
{
    g_want_adv = false;
    stop_adv();
}

#endif
