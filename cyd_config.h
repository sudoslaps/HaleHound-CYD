// =============================================================================
// cyd_config.h  —  HaleHound-CYD  •  E32R40T 4.0" Port
// =============================================================================
//
// Target Board : lcdwiki.com  4.0inch ESP32-32E Display  (SKU: E32R40T)
// Module       : ESP32-32E (ESP32-WROOM-32E)
// Display      : ST7796S  320x480  (portrait)
// Touch        : XPT2046  resistive — SHARED SPI bus with display (HSPI)
// Original     : ESP32-2432S028  2.8"  ILI9341  320x240
//
// -------------------------  KEY DIFFERENCES FROM 2.8" CYD  ------------------
//
//  GPIO 27   : was CC1101 CS   → now BACKLIGHT (digital HI=on)
//  GPIO 21   : was TFT BL PWM → now SPI-periph CS (free IO) ← use for CC1101 CS
//  GPIO 22   : was CC1101 GDO0→ now RGB RED LED (common-anode, LOW=on)
//  GPIO 4    : was NRF24 CSN  → now AUDIO ENABLE (LOW=enable)
//  GPIO 25   : was touch CLK  → now I2C SCL (free for us)
//  GPIO 32   : was touch MOSI → now I2C SDA (free for us)
//  Touch SPI : no longer bit-banged on its own pins;
//              shares HSPI bus (GPIO 14 CLK / 13 MOSI / 12 MISO) with display
//
//  Resolution : 320x480  (was 320x240). All UI zone coords updated below.
//
// =============================================================================

#pragma once

// ---------------------------------------------------------------------------
// Board selection — one must be defined
// ---------------------------------------------------------------------------
// #define CYD_28       // ESP32-2432S028  2.8"  ILI9341  320x240
// #define CYD_35       // ESP32-3248S035  3.5"  ST7796   480x320
#define CYD_E32R40T     // lcdwiki E32R40T 4.0"  ST7796S  320x480  ← ACTIVE

// ---------------------------------------------------------------------------
// Display geometry
// ---------------------------------------------------------------------------
#ifdef CYD_E32R40T
  #define TFT_WIDTH   320
  #define TFT_HEIGHT  480
#elif defined(CYD_35)
  #define TFT_WIDTH   480
  #define TFT_HEIGHT  320
#else
  // CYD_28 default
  #define TFT_WIDTH   320
  #define TFT_HEIGHT  240
#endif

// ---------------------------------------------------------------------------
// TFT HSPI bus (unchanged from original — same physical pins on E32R40T)
// ---------------------------------------------------------------------------
#define TFT_MISO_PIN   12
#define TFT_MOSI_PIN   13
#define TFT_SCLK_PIN   14
#define TFT_CS_PIN     15
#define TFT_DC_PIN      2
// TFT RST tied to ESP32 EN pin (hardware reset only)

// ---------------------------------------------------------------------------
// Backlight
// NOTE: E32R40T backlight is GPIO 27, digital HIGH = backlight ON.
//       The original CYD used GPIO 21 with PWM. The E32R40T does NOT expose
//       a PWM-dimming path to the LED driver, so we use digital on/off here.
//       Brightness "dimming" is achieved with fast on/off toggling (SW PWM)
//       or simply accepted as binary. Adjust LEDC channel if you add hw pwm.
// ---------------------------------------------------------------------------
#ifdef CYD_E32R40T
  #define TFT_BL_PIN       27      // HIGH = backlight on
  #define TFT_BL_ON_LEVEL   HIGH
  #define TFT_BL_PWM_CHAN   0      // LEDC channel for SW-PWM if desired
#else
  #define TFT_BL_PIN       21
  #define TFT_BL_ON_LEVEL  HIGH
  #define TFT_BL_PWM_CHAN   0
#endif

// ---------------------------------------------------------------------------
// Touch  — XPT2046 on SHARED HSPI (same CLK/MOSI/MISO as display)
//          CS and IRQ are board-specific
// ---------------------------------------------------------------------------
#define TOUCH_CS_PIN    33
#define TOUCH_IRQ_PIN   36

// E32R40T touch shares HSPI; set TOUCH_USES_HSPI so the touch driver
// skips bit-bang init and uses the existing SPI bus handle.
#define TOUCH_USES_HSPI   1

// Original 2.8" CYD bit-bang touch pins (unused on E32R40T but kept for ref)
// #define TOUCH_CLK_PIN   25
// #define TOUCH_MOSI_PIN  32
// #define TOUCH_MISO_PIN  39

// ---------------------------------------------------------------------------
// VSPI — SD card + CC1101 + NRF24  (unchanged)
// ---------------------------------------------------------------------------
#define VSPI_SCK_PIN    18
#define VSPI_MOSI_PIN   23
#define VSPI_MISO_PIN   19

// SD card (built-in slot)
#define SD_CS_PIN        5

// ---------------------------------------------------------------------------
// CC1101 SubGHz Radio
// GPIO 27 is now backlight on E32R40T, so CS moves to GPIO 21.
// GDO0 (TX to radio) moved from GPIO 22 (now RGB RED) to GPIO 25 (free).
// GDO2 (RX from radio) stays on GPIO 35 (input-only, correct).
// ---------------------------------------------------------------------------
#ifdef CYD_E32R40T
  #define CC1101_CS_PIN    21   // was 27 (now backlight)
  #define CC1101_GDO0_PIN  25   // was 22 (now RGB RED LED)
  #define CC1101_GDO2_PIN  35   // unchanged (input-only GPIO)
#else
  #define CC1101_CS_PIN    27
  #define CC1101_GDO0_PIN  22
  #define CC1101_GDO2_PIN  35
#endif

// ---------------------------------------------------------------------------
// NRF24L01+PA+LNA
// GPIO 4 is now AUDIO ENABLE on E32R40T. CSN moves to GPIO 32 (free I2C SDA).
// CE stays GPIO 16, IRQ stays GPIO 17.
// ---------------------------------------------------------------------------
#ifdef CYD_E32R40T
  #define NRF24_CSN_PIN    32   // was 4 (now audio enable)
  #define NRF24_CE_PIN     16   // unchanged (was RGB GREEN LED — still repurposed)
  #define NRF24_IRQ_PIN    17   // unchanged (was RGB BLUE LED  — still repurposed)
#else
  #define NRF24_CSN_PIN     4
  #define NRF24_CE_PIN     16
  #define NRF24_IRQ_PIN    17
#endif

// ---------------------------------------------------------------------------
// RGB LED  (common-anode: LOW = ON, HIGH = OFF)
// E32R40T: RED=GPIO22, GREEN=GPIO16, BLUE=GPIO17
// NOTE: GREEN and BLUE are repurposed for NRF24 CE/IRQ (same as original).
//       RED is now on GPIO 22 (was CC1101 GDO0 on 2.8" CYD).
//       With NRF24 connected, all LED channels are consumed.
// ---------------------------------------------------------------------------
#ifdef CYD_E32R40T
  #define RGB_RED_PIN     22
  #define RGB_GREEN_PIN   16
  #define RGB_BLUE_PIN    17
#else
  // Original CYD had no dedicated red; green=16, blue=17, red=4
  #define RGB_RED_PIN      4
  #define RGB_GREEN_PIN   16
  #define RGB_BLUE_PIN    17
#endif

// ---------------------------------------------------------------------------
// Audio  (E32R40T has dedicated audio enable + DAC)
// GPIO 4  = FM8002E amplifier enable (LOW = enabled)
// GPIO 26 = DAC audio output
// ---------------------------------------------------------------------------
#define AUDIO_EN_PIN    4    // LOW = amp enabled  (E32R40T specific)
#define AUDIO_DAC_PIN  26

// ---------------------------------------------------------------------------
// GPS / Serial
// ---------------------------------------------------------------------------
#define GPS_RX_PIN       3   // P1 serial header RX  (shared with USB Serial RX)
#define GPS_TX_PIN       1   // P1 serial header TX

// ---------------------------------------------------------------------------
// Misc GPIO
// ---------------------------------------------------------------------------
#define BOOT_BTN_PIN     0   // Active LOW
#define BATTERY_ADC_PIN 34   // Input-only, ADC
// GPIO 35, 39 — input-only (used for CC1101 GDO2 and was touch MISO)

// ---------------------------------------------------------------------------
// Feature flags
// ---------------------------------------------------------------------------
#define CYD_HAS_CC1101        1
#define CYD_HAS_NRF24         1
#define CYD_HAS_GPS           1
#define CYD_HAS_SDCARD        1
#define CYD_HAS_RGB_LED       0   // LED pins consumed by NRF24 CE/IRQ + CC1101 GDO0
#define CYD_HAS_SPEAKER       1   // E32R40T has audio amp — GPIO 4 enable + GPIO 26 DAC
#define CYD_HAS_PCF8574       0
#define CYD_HAS_SERIAL_MON    1
#define CYD_HAS_AUDIO_ENABLE  1   // New flag: amp has an enable pin

// ---------------------------------------------------------------------------
// Touch UI zones  —  updated for 320x480 portrait
//
//  Original 2.8" (320x240):
//    UP   x:0-80   y:0-60
//    BACK x:160-240 y:0-60
//    DOWN x:0-80   y:260-320
//    SELECT center ~(80-160, 130-190) i.e. middle third
//
//  E32R40T (320x480):
//    Status bar: y: 0-40
//    UP zone:    x:0-100  y:40-160
//    BACK zone:  x:220-320 y:40-160
//    DOWN zone:  x:0-100  y:360-480
//    SELECT:     x:80-240  y:180-300  (centre of screen)
// ---------------------------------------------------------------------------
#ifdef CYD_E32R40T
  #define SCREEN_W           TFT_WIDTH    // 320
  #define SCREEN_H           TFT_HEIGHT   // 480
  #define STATUS_BAR_H       40
  #define TOUCH_UP_X1        0
  #define TOUCH_UP_X2        100
  #define TOUCH_UP_Y1        40
  #define TOUCH_UP_Y2        160
  #define TOUCH_DOWN_X1      0
  #define TOUCH_DOWN_X2      100
  #define TOUCH_DOWN_Y1      360
  #define TOUCH_DOWN_Y2      480
  #define TOUCH_BACK_X1      220
  #define TOUCH_BACK_X2      320
  #define TOUCH_BACK_Y1      40
  #define TOUCH_BACK_Y2      160
  #define TOUCH_SELECT_X1    80
  #define TOUCH_SELECT_X2    240
  #define TOUCH_SELECT_Y1    180
  #define TOUCH_SELECT_Y2    300
  // Main menu grid: 2 columns x 4 rows  (was 2x4 on 320x240, fits better on 480px tall)
  #define MENU_COLS          2
  #define MENU_ROWS          4
  #define MENU_CELL_W        (SCREEN_W / MENU_COLS)   // 160
  #define MENU_CELL_H        ((SCREEN_H - STATUS_BAR_H) / MENU_ROWS)  // 110
  // List items in submenus
  #define LIST_ITEM_H        48
  #define LIST_VISIBLE_ITEMS 8    // items visible without scrolling (480-40)/48 ≈ 9
#else
  // 2.8" / 3.5" defaults (unchanged from original)
  #define SCREEN_W           TFT_WIDTH
  #define SCREEN_H           TFT_HEIGHT
  #define STATUS_BAR_H       20
  #define TOUCH_UP_X1        0
  #define TOUCH_UP_X2        80
  #define TOUCH_UP_Y1        0
  #define TOUCH_UP_Y2        60
  #define TOUCH_DOWN_X1      0
  #define TOUCH_DOWN_X2      80
  #define TOUCH_DOWN_Y1      260
  #define TOUCH_DOWN_Y2      320
  #define TOUCH_BACK_X1      160
  #define TOUCH_BACK_X2      240
  #define TOUCH_BACK_Y1      0
  #define TOUCH_BACK_Y2      60
  #define TOUCH_SELECT_X1    80
  #define TOUCH_SELECT_X2    160
  #define TOUCH_SELECT_Y1    130
  #define TOUCH_SELECT_Y2    190
  #define MENU_COLS          2
  #define MENU_ROWS          4
  #define MENU_CELL_W        (SCREEN_W / MENU_COLS)
  #define MENU_CELL_H        ((SCREEN_H - STATUS_BAR_H) / MENU_ROWS)
  #define LIST_ITEM_H        28
  #define LIST_VISIBLE_ITEMS 6
#endif
