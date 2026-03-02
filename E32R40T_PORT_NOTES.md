# HaleHound-CYD — E32R40T 4.0" Port Notes

## Target Board

**lcdwiki 4.0inch ESP32-32E Display — SKU: E32R40T**
- ESP32-32E module (WROOM-32E)
- ST7796S display driver, 320×480 portrait
- XPT2046 resistive touch
- Built-in MicroSD slot
- RGB LED (common-anode), audio amp (FM8002E), battery charger (TP4054)

---

## Files Changed / Added in This Port

| File | What changed |
|------|-------------|
| `cyd_config.h` | Complete rewrite for E32R40T pinout, 320×480 UI zones |
| `User_Setup.h` | ST7796S driver, updated BL pin (GPIO27), no RST pin |
| `touch_buttons.cpp` | Switched from bit-bang CYD28_TouchscreenR → XPT2046_Touchscreen on shared HSPI; updated cal routine for 480px height |
| `touch_buttons.h` | Same public API, implementation details hidden |
| `spi_manager.cpp` | Updated CS pin assignments (CC1101 CS→21, NRF24 CSN→32) |
| `spi_manager.h` | Unchanged API |
| `platformio.ini` | New `esp32-e32r40t` environment, removed CYD28_TouchscreenR lib |

**All other source files are unchanged.** The rest of the firmware (wifi attacks, bluetooth, NRF24, CC1101, EAPOL, karma, wardriving, GPS, serial monitor, OTA, etc.) compiles against the new `cyd_config.h` pin definitions without modification.

---

## Critical Pin Differences (2.8" CYD → E32R40T)

| GPIO | Original 2.8" CYD | E32R40T |
|------|-------------------|---------|
| **4** | NRF24 CSN | **Audio amplifier ENABLE** (LOW=on) |
| **21** | TFT Backlight PWM | SPI peripheral CS (free) → now CC1101 CS |
| **22** | CC1101 GDO0 (TX) | **RGB RED LED** (common-anode) |
| **25** | Touch CLK (bit-bang) | I2C SCL → now CC1101 GDO0 |
| **27** | CC1101 CS | **TFT Backlight** (HIGH=on) |
| **32** | Touch MOSI (bit-bang) | I2C SDA → now NRF24 CSN |
| Touch SPI | Bit-banged (GPIO 25/32/39/33) | Shared HSPI (GPIO 14/13/12/33/36) |
| Resolution | 320×240 | **320×480** |

### Resulting Radio Pin Map (E32R40T)

| Peripheral | Signal | GPIO |
|-----------|--------|------|
| CC1101 | CS | 21 |
| CC1101 | GDO0 (TX to radio) | 25 |
| CC1101 | GDO2 (RX from radio) | 35 |
| NRF24 | CSN | 32 |
| NRF24 | CE | 16 |
| NRF24 | IRQ | 17 |
| All radios | SCK/MOSI/MISO | 18/23/19 (VSPI) |

### Audio (E32R40T only)

The E32R40T has a real audio amplifier (FM8002E) with an enable pin:

```cpp
#define AUDIO_EN_PIN   4    // LOW = amp on
#define AUDIO_DAC_PIN  26   // DAC output
```

The `CYD_HAS_SPEAKER` flag is set to `1` (speaker is actually usable on this board). The original 2.8" CYD had GPIO 26 connected through an 8002A amp that couldn't be reliably bypassed for serial monitor use. On the E32R40T, the serial monitor uses the P1 header (GPIO 3/1) as before; GPIO 26 is the DAC and does not conflict.

---

## Touch Library Change

The original firmware used `CYD28_TouchscreenR` — a custom bit-banged XPT2046 driver needed because the 2.8" CYD ran the touch controller on a completely separate SPI bus (GPIO 25/32/39/33).

On the E32R40T the XPT2046 shares the display's HSPI bus (GPIO 14/13/12), with only the CS (GPIO 33) and IRQ (GPIO 36) lines separate. The standard `paulstoffregen/XPT2046_Touchscreen` library handles this correctly when given the `SPIClass` handle:

```cpp
SPIClass hspi(HSPI);
hspi.begin(TFT_SCLK_PIN, TFT_MISO_PIN, TFT_MOSI_PIN);
XPT2046_Touchscreen ts(TOUCH_CS_PIN, TOUCH_IRQ_PIN);
ts.begin(hspi);
```

> **Note:** Because the touch now shares the display bus, the display CS (GPIO 15) must be HIGH during touch reads, and vice versa. TFT_eSPI handles its own CS. The XPT2046 library handles its CS. As long as both libraries use `beginTransaction`/`endTransaction` correctly (they do), bus sharing is safe.

---

## Display Resolution — UI Impact

The move from 320×240 to 320×480 means the screen is **twice as tall**. The main menu (2×4 icon grid) now has cells of 160×110px (was 160×55px). Submenu list items are 48px tall (was 28px), fitting ~9 items without scrolling.

Touch zones are defined in `cyd_config.h` and used throughout all `.cpp` files via the macros (`TOUCH_UP_Y1`, etc.). No hardcoded coordinates exist in attack modules — they all call `isUpZone()`, `isDownZone()`, etc.

**The splash/boot graphic (`skull_bg.h`) may need to be regenerated** at 320×480 if it was compiled as a fixed-size bitmap. Scale it up or replace with a new asset.

---

## Build Instructions

```bash
# Build for E32R40T
pio run -e esp32-e32r40t

# Flash
pio run -e esp32-e32r40t --target upload

# Build + flash + monitor
pio run -e esp32-e32r40t --target upload && pio device monitor -b 115200
```

On first boot, touch calibration runs automatically (same as original).  
If display is upside-down: **Settings → Rotation**.

---

## What Doesn't Change

Everything else is identical to the original v2.9.0 firmware:

- All WiFi attack modules
- All Bluetooth / BLE modules
- NRF24 attack modules
- CC1101 SubGHz modules
- EAPOL capture / Karma / Wardriving
- GPS module
- Serial monitor
- OTA firmware update from SD
- SPI bus arbitration logic
- EEPROM layout (brightness, rotation, color swap, screen timeout)

---

## Known Issues (E32R40T specific)

| Issue | Notes |
|-------|-------|
| Skull background bitmap size | `skull_bg.h` may need regenerating at 320×480 |
| Audio enable pin conflict | GPIO 4 is AUDIO EN — if NRF24 CSN (now GPIO 32) has any init issue, verify GPIO 4 stays HIGH during NRF24 transactions |
| Backlight PWM | GPIO 27 backlight responds to LEDC but verify the board's LED driver doesn't clamp below a certain duty cycle |
| Touch bus sharing | On very fast display writes, XPT2046 reads occasionally show spurious touches — add a 5ms debounce after display updates if observed |
