#ifndef SLAB_BOARD_H
#define SLAB_BOARD_H

/* Seeed XIAO nRF52840 + Waveshare 2.9" B/W SPI. See firmware/PINMAP.md. */

#define SLAB_PIN_EPD_DIN 10 /* D10 MOSI */
#define SLAB_PIN_EPD_SCK 8  /* D8  SCK  */
#define SLAB_PIN_EPD_CS 7   /* D7  CS   */
#define SLAB_PIN_EPD_DC 6   /* D6  DC   */
#define SLAB_PIN_EPD_RST 3  /* D3  RST  */
#define SLAB_PIN_EPD_BUSY 1 /* D1  BUSY high=busy */

#define SLAB_PIN_BTN_PLUS 2  /* D2 */
#define SLAB_PIN_BTN_MINUS 4 /* D4 */
#define SLAB_PIN_BTN_PUTT 5  /* D5 — present, ignored by sequential lock */
#define SLAB_PIN_BTN_MODE 0  /* D0 */
#define SLAB_PIN_BTN_NEXT 9  /* D9 */

/* Optional quadrature dial (unpopulated on P1). Same +/- path as pads. */
#define SLAB_PIN_ENC_A (-1)
#define SLAB_PIN_ENC_B (-1)

#define SLAB_EPD_DRIVER "GxEPD2_290_T94_V2"
#define SLAB_EPD_CONTROLLER "SSD1680"
#define SLAB_EPD_PANEL "Waveshare 2.9 B/W V2/V3 (GDEM029T94 family)"

#endif
