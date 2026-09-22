#if !defined(SLAB_FACTORY_PROVISION)
#error "This sketch is the factory provisioner. Build env:xiaoble_provision only."
#endif

/*
 * Factory/service provisioner for the Slab XIAO bench.
 *
 * This is not the scoring firmware and it is not the production boot path.
 * Scoring startup (firmware/src/main.cpp) calls
 * InternalFS.Adafruit_LittleFS::begin() and never formats.
 *
 * Do not call InternalFileSystem::begin(). In framework
 * 3ae21c3c6c544fed6e949f00986324dd95fb2b89 that override erases and formats
 * when mount fails. Erase here happens only after the operator types
 * "ERASE SLAB FS" on Serial.
 *
 * Region bounds are copied from that framework's InternalFileSystem.cpp
 * (NRF52840_XXAA) and flash_nrf5x.c (BOOTLOADER_ADDR 0xF4000). The last
 * erased page ends at the bootloader and does not include it.
 */

#include <Arduino.h>
#include <string.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include <Adafruit_TinyUSB.h>

extern "C" {
#include "flash/flash_nrf5x.h"
}

#ifndef NRF52840_XXAA
#error "Slab provisioner is for the Seeed XIAO nRF52840 (NRF52840_XXAA) only."
#endif

static const uint32_t kLfsAddr = 0x000ED000u;
static const uint32_t kPageSize = 4096u;
static const uint32_t kPageCount = 7u;
static const uint32_t kBootloaderAddr = 0x000F4000u;
static_assert(kPageSize == FLASH_NRF52_PAGE_SIZE, "page size must match flash_nrf5x.h");
static_assert(kLfsAddr % 4096u == 0u, "LittleFS region must be page aligned");
static_assert(kLfsAddr + (kPageCount * kPageSize) == kBootloaderAddr,
              "LittleFS region must end where the bootloader begins");

static const char kErasePhrase[] = "ERASE SLAB FS";
static const char *const kWarning[] = {
    "======== SLAB FACTORY PROVISIONER ========",
    "NOT SCORING FIRMWARE. NOT THE PRODUCTION BOOT PATH.",
    "WARNING: THE NEXT CONFIRMED STEP ERASES INTERNAL LITTLEFS.",
    "Region 0x000ED000 + 28672 bytes (7 pages of 4096).",
    "This destroys /slab_store.0, /slab_store.1, and /slab_round.bin.",
    "It does not erase the application, SoftDevice, or bootloader.",
    "A USB-C data cable is required. A charge-only cable cannot confirm this step.",
    "Reset now to leave flash UNCHANGED.",
    "No erase happens until this exact line is received:",
    "ERASE SLAB FS",
    "==========================================",
};
static const char *const kJournalPaths[] = {
    "/slab_store.0",
    "/slab_store.1",
    "/slab_round.bin",
};

static bool g_failed;

static void leds_off(void)
{
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    digitalWrite(LED_RED, !LED_STATE_ON);
    digitalWrite(LED_GREEN, !LED_STATE_ON);
    digitalWrite(LED_BLUE, !LED_STATE_ON);
}

static void wait_for_serial(void)
{
    while (!Serial) {
        digitalWrite(LED_RED, ((millis() / 200u) & 1u) ? LED_STATE_ON : !LED_STATE_ON);
        delay(10);
    }
    digitalWrite(LED_RED, !LED_STATE_ON);
}

static void print_warning(void)
{
    Serial.println();
    for (unsigned i = 0; i < sizeof(kWarning) / sizeof(kWarning[0]); i++) {
        Serial.println(kWarning[i]);
    }
    Serial.flush();
}

static void report_mount(bool mounted)
{
    Serial.print("Mount-only check (no erase yet): ");
    Serial.println(mounted ? "MOUNTED" : "NOT MOUNTED");
    if (!mounted) {
        Serial.println("Flash is unchanged.");
        return;
    }
    for (unsigned i = 0; i < sizeof(kJournalPaths) / sizeof(kJournalPaths[0]); i++) {
        Serial.print(kJournalPaths[i]);
        Serial.println(InternalFS.exists(kJournalPaths[i]) ? " present" : " absent");
    }
}

/* True only for the exact phrase. A wrong line, an empty line, or a line
 * that does not fit in the buffer returns false and does not erase. */
static bool read_exact_phrase(void)
{
    char buf[32];
    size_t n = 0;
    bool overflow = false;
    Serial.println("TYPE THIS EXACT LINE, THEN ENTER:");
    Serial.println(kErasePhrase);
    Serial.flush();
    while (true) {
        while (!Serial.available()) {
            digitalWrite(LED_RED, ((millis() / 200u) & 1u) ? LED_STATE_ON : !LED_STATE_ON);
            delay(10);
        }
        int raw = Serial.read();
        if (raw < 0) continue;
        char c = (char)raw;
        if (c == '\r' || c == '\n') {
            if (n == 0 && !overflow) continue; /* ignore the second half of CRLF */
            buf[n] = '\0';
            Serial.print("Received: [");
            Serial.print(overflow ? "TOO LONG" : buf);
            Serial.println("]");
            Serial.flush();
            return !overflow && strcmp(buf, kErasePhrase) == 0;
        }
        if (n + 1 >= sizeof(buf)) overflow = true;
        else buf[n++] = c;
    }
}

static bool journal_absent(void)
{
    bool ok = true;
    for (unsigned i = 0; i < sizeof(kJournalPaths) / sizeof(kJournalPaths[0]); i++) {
        bool present = InternalFS.exists(kJournalPaths[i]);
        Serial.print(kJournalPaths[i]);
        Serial.println(present ? " present" : " absent");
        if (present) ok = false;
    }
    return ok;
}

/* Called only after read_exact_phrase() returned true. */
static bool erase_and_format(void)
{
    Serial.println("ERASE CONFIRMED. Erasing internal LittleFS now.");
    Serial.println("Do not disconnect USB.");
    Serial.flush();
    digitalWrite(LED_RED, LED_STATE_ON);
    digitalWrite(LED_GREEN, !LED_STATE_ON);

    InternalFS.end();
    flash_nrf5x_flush();

    for (uint32_t page = 0; page < kPageCount; page++) {
        uint32_t addr = kLfsAddr + page * kPageSize;
        if (addr < kLfsAddr || addr >= kBootloaderAddr || (addr % kPageSize) != 0u) {
            Serial.println("Erase address rejected.");
            return false;
        }
        Serial.print("Erasing page 0x");
        Serial.println(addr, HEX);
        Serial.flush();
        if (!flash_nrf5x_erase(addr)) {
            Serial.println("Page erase failed.");
            return false;
        }
    }

    /* format() uses lfs_format. The filesystem is not mounted, so this does
     * not remount. Mount-only begin() below is the production call. */
    if (!InternalFS.format()) {
        Serial.println("LittleFS format failed.");
        return false;
    }
    flash_nrf5x_flush();

    if (!InternalFS.Adafruit_LittleFS::begin()) {
        Serial.println("Mount-only begin failed after format.");
        return false;
    }
    bool empty = journal_absent();
    InternalFS.end();
    if (!empty) {
        Serial.println("Formatted filesystem still has a journal or legacy file.");
        return false;
    }
    return true;
}

static void finish_refused(void)
{
    Serial.println("REFUSED");
    Serial.println("Flash was not erased.");
    Serial.println("Reset the board to try again.");
    Serial.flush();
    digitalWrite(LED_RED, LED_STATE_ON);
    digitalWrite(LED_GREEN, !LED_STATE_ON);
}

static void finish_failed(void)
{
    g_failed = true;
    Serial.println("PROVISION FAILED");
    Serial.println("The LittleFS region may be erased or only partly erased.");
    Serial.println("Do not treat this board as provisioned.");
    Serial.println("Run this provisioner again and wait for PROVISION OK.");
    Serial.println("Scoring firmware stays fail-closed and will show STORAGE ERROR until mount works.");
    Serial.flush();
    digitalWrite(LED_RED, LED_STATE_ON);
    digitalWrite(LED_GREEN, !LED_STATE_ON);
}

static void finish_ok(void)
{
    Serial.println("PROVISION OK");
    Serial.println("Blank LittleFS mounted with the same mount-only call production uses.");
    Serial.println("Journal files were not written by this tool.");
    Serial.println("Scoring firmware first boot creates the empty store when both slots are absent.");
    Serial.println("Flash env xiaoble next. Panel text is the scoring-firmware evidence.");
    Serial.flush();
    digitalWrite(LED_RED, !LED_STATE_ON);
    digitalWrite(LED_GREEN, LED_STATE_ON);
}

void setup(void)
{
    leds_off();
    Serial.begin(115200);
    wait_for_serial();
    print_warning();

    bool mounted = InternalFS.Adafruit_LittleFS::begin();
    report_mount(mounted);
    if (mounted) InternalFS.end();
    Serial.println("Flash is unchanged.");
    Serial.flush();

    if (!read_exact_phrase()) {
        finish_refused();
        return;
    }
    if (!erase_and_format()) {
        finish_failed();
        return;
    }
    finish_ok();
}

void loop(void)
{
    /* setup() already finished. Never erase from loop, including after reset
     * of the USB port that does not re-enter setup. A chip reset runs setup()
     * again and waits for a new phrase. */
    if (g_failed) digitalWrite(LED_RED, LED_STATE_ON);
    delay(1000);
}
