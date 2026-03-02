// =============================================================================
// User_Setup.h  —  TFT_eSPI driver config for HaleHound-CYD E32R40T port
// =============================================================================
// Copy this file into your TFT_eSPI library folder, replacing the existing
// User_Setup.h, OR set in platformio.ini:
//   build_flags = -DUSER_SETUP_LOADED -DUSER_SETUP_INFO="HaleHound E32R40T"
// and point the build system here.
//
// Target: lcdwiki 4.0" ESP32-32E (E32R40T)
//   Driver  : ST7796S
//   Bus     : SPI HSPI  (GPIO 14 CLK / 13 MOSI / 12 MISO)
//   CS      : GPIO 15
//   DC      : GPIO 2
//   RST     : GPIO EN  (hardware only — no software reset pin)
//   BL      : GPIO 27  (digital HIGH = on; PWM optional via LEDC)
//   Touch   : XPT2046  on SAME HSPI bus, CS=GPIO33, IRQ=GPIO36
// =============================================================================

#define USER_SETUP_LOADED   1
#define USER_SETUP_INFO     "HaleHound-CYD E32R40T 4.0in ST7796S"

// ---------------------------------------------------------------------------
// Driver
// ---------------------------------------------------------------------------
#define ST7796_DRIVER       // ST7796 / ST7796S  (320x480)

// ---------------------------------------------------------------------------
// Display dimensions (portrait)
// ---------------------------------------------------------------------------
#define TFT_WIDTH  320
#define TFT_HEIGHT 480

// ---------------------------------------------------------------------------
// HSPI pin mapping
// ---------------------------------------------------------------------------
#define TFT_MISO  12
#define TFT_MOSI  13
#define TFT_SCLK  14
#define TFT_CS    15    // Chip select
#define TFT_DC     2    // Data/Command
#define TFT_RST   -1    // Tied to ESP32 EN; -1 = not software-controlled
#define TFT_BL    27    // Backlight — HIGH = on

// ---------------------------------------------------------------------------
// Touch controller (XPT2046)
// Shares HSPI with the display on E32R40T
// ---------------------------------------------------------------------------
#define TOUCH_CS  33    // XPT2046 chip select

// ---------------------------------------------------------------------------
// SPI frequency
// ---------------------------------------------------------------------------
#define SPI_FREQUENCY       40000000    // 40 MHz for display
#define SPI_READ_FREQUENCY   6000000    //  6 MHz read-back
#define SPI_TOUCH_FREQUENCY  2500000    //  2.5 MHz touch (lower = more stable)

// ---------------------------------------------------------------------------
// Colour depth
// ---------------------------------------------------------------------------
// ST7796S supports RGB666 (18-bit) natively but TFT_eSPI defaults to RGB565
// which is fine and avoids 3-byte pixel writes.
// #define TFT_RGB_ORDER TFT_BGR  // uncomment if colours appear inverted

// ---------------------------------------------------------------------------
// Fonts  (match originals used in HaleHound)
// ---------------------------------------------------------------------------
#define LOAD_GLCD    // Font 1 (built-in)
#define LOAD_FONT2   // Font 2
#define LOAD_FONT4   // Font 4
#define LOAD_FONT6   // Font 6
#define LOAD_FONT7   // Font 7
#define LOAD_FONT8   // Font 8
#define LOAD_GFXFF   // FreeFont support
#define SMOOTH_FONT

// ---------------------------------------------------------------------------
// DMA  (optional, improves frame rate)
// ---------------------------------------------------------------------------
#define SUPPORT_TRANSACTIONS
// #define ESP32_DMA  // Uncomment to enable DMA transfers
