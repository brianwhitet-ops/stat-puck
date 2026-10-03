#ifndef SLAB_BOARD_H
#define SLAB_BOARD_H

/* Seeed XIAO nRF52840 (not Sense) + Waveshare 2.9" B/W SPI.
 * Current bench wiring. Display pixels stay on stock GxEPD2_290_T94_V2.
 * See firmware/PINMAP.md. */

#define SLAB_PIN_EPD_DIN 10  /* D10 MOSI */
#define SLAB_PIN_EPD_SCK 8   /* D8  SCK  */
#define SLAB_PIN_EPD_CS 7    /* D7  CS   */
#define SLAB_PIN_EPD_DC 5    /* D5  DC   */
#define SLAB_PIN_EPD_RST 4   /* D4  RST  */
#define SLAB_PIN_EPD_BUSY 3  /* D3  BUSY high=busy */
/* Panel has no MISO. Keep it off D9 so NEXT can use that pin. */
#define SLAB_PIN_EPD_MISO 1  /* D1 */

#define SLAB_PIN_BTN_PLUS 2    /* D2 */
#define SLAB_PIN_BTN_MINUS 6   /* D6 */
#define SLAB_PIN_BTN_PUTT 0xFF /* dropped from this UX; do not read a pad */
#define SLAB_PIN_BTN_MODE 0    /* D0 — short: back / reopen; hold ~2s: sleep */
#define SLAB_PIN_BTN_NEXT 9    /* D9 */

/* Optional quadrature dial (unpopulated on P1). Same +/- path as pads. */
#define SLAB_PIN_ENC_A (-1)
#define SLAB_PIN_ENC_B (-1)

#define SLAB_EPD_DRIVER "GxEPD2_290_T94_V2"
#define SLAB_EPD_CONTROLLER "SSD1680"
#define SLAB_EPD_PANEL "Waveshare 2.9 B/W V2/V3 (GDEM029T94 family)"

#endif
