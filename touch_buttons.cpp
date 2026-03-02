// =============================================================================
// touch_buttons.cpp  —  E32R40T port
// =============================================================================
// Key changes from original CYD_28 version:
//  1. Touch XPT2046 now SHARES the HSPI bus with the display.
//     No more bit-banged custom driver (CYD28_TouchscreenR).
//     Use the standard XPT2046_Touchscreen library instead.
//  2. All touch zone coordinates updated for 320x480.
//  3. Backlight control uses GPIO 27 (digital) or LEDC SW-PWM.
//  4. Calibration EEPROM offsets unchanged.
// =============================================================================

#include "touch_buttons.h"
#include "cyd_config.h"
#include "shared.h"
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <EEPROM.h>

// ---------------------------------------------------------------------------
// On E32R40T the XPT2046 shares the display's HSPI bus.
// We pass SPIClass hspi to the library so it uses the existing bus.
// ---------------------------------------------------------------------------
extern SPIClass hspi;   // defined in HaleHound-CYD.ino

static XPT2046_Touchscreen ts(TOUCH_CS_PIN, TOUCH_IRQ_PIN);

// Calibration — raw ADC to pixel mapping
// Defaults work for most boards; overwritten by calibration routine.
static int16_t calX0 = 300, calX1 = 3800;   // raw min / max in X
static int16_t calY0 = 300, calY1 = 3800;   // raw min / max in Y
static bool    calFlipX = false;
static bool    calFlipY = false;

// EEPROM layout (unchanged from original so existing calibrations survive
// on boards that were previously running the 2.8" firmware — though the
// cal values themselves will need redoing for this new screen)
#define EEPROM_SIZE         32
#define EEPROM_CAL_VALID    0   // 1 byte  (0xAB = valid)
#define EEPROM_CAL_X0       1   // 2 bytes
#define EEPROM_CAL_X1       3   // 2 bytes
#define EEPROM_CAL_Y0       5   // 2 bytes
#define EEPROM_CAL_Y1       7   // 2 bytes
#define EEPROM_CAL_FLIPX    9   // 1 byte
#define EEPROM_CAL_FLIPY   10   // 1 byte
#define EEPROM_BRIGHTNESS  11   // 1 byte
#define EEPROM_ROTATION    12   // 1 byte
#define EEPROM_SWAP_CLR    13   // 1 byte
#define EEPROM_TIMEOUT     14   // 1 byte

// Active screen rotation (affects touch coordinate transform)
static uint8_t screenRotation = 0;  // 0 = portrait, default

// ---------------------------------------------------------------------------
// Backlight  —  E32R40T: GPIO 27 digital / optional LEDC SW-PWM
// ---------------------------------------------------------------------------
static uint8_t currentBrightness = 200;   // 0-255

void backlightInit() {
#ifdef CYD_E32R40T
    // Optionally set up LEDC for PWM dimming on GPIO 27
    ledcSetup(TFT_BL_PWM_CHAN, 5000, 8);        // 5 kHz, 8-bit
    ledcAttachPin(TFT_BL_PIN, TFT_BL_PWM_CHAN);
    ledcWrite(TFT_BL_PWM_CHAN, currentBrightness);
#else
    ledcSetup(TFT_BL_PWM_CHAN, 5000, 8);
    ledcAttachPin(TFT_BL_PIN, TFT_BL_PWM_CHAN);
    ledcWrite(TFT_BL_PWM_CHAN, currentBrightness);
#endif
}

void setBrightness(uint8_t val) {
    currentBrightness = val;
    ledcWrite(TFT_BL_PWM_CHAN, val);
}

uint8_t getBrightness() { return currentBrightness; }

// ---------------------------------------------------------------------------
// Backlight save/load
// ---------------------------------------------------------------------------
void saveBrightness() {
    EEPROM.write(EEPROM_BRIGHTNESS, currentBrightness);
    EEPROM.commit();
}

void loadBrightness() {
    uint8_t v = EEPROM.read(EEPROM_BRIGHTNESS);
    if (v < 10 || v > 255) v = 200;
    currentBrightness = v;
    setBrightness(v);
}

// ---------------------------------------------------------------------------
// Touch init — shared HSPI
// ---------------------------------------------------------------------------
void touchInit() {
    // hspi must already be begun by the time this is called.
    // XPT2046_Touchscreen is told to use hspi.
    ts.begin(hspi);
    ts.setRotation(screenRotation);
}

// ---------------------------------------------------------------------------
// Calibration save / load
// ---------------------------------------------------------------------------
static void saveCalibration() {
    EEPROM.write(EEPROM_CAL_VALID, 0xAB);
    EEPROM.put(EEPROM_CAL_X0, calX0);
    EEPROM.put(EEPROM_CAL_X1, calX1);
    EEPROM.put(EEPROM_CAL_Y0, calY0);
    EEPROM.put(EEPROM_CAL_Y1, calY1);
    EEPROM.write(EEPROM_CAL_FLIPX, calFlipX ? 1 : 0);
    EEPROM.write(EEPROM_CAL_FLIPY, calFlipY ? 1 : 0);
    EEPROM.commit();
}

bool loadCalibration() {
    EEPROM.begin(EEPROM_SIZE);
    if (EEPROM.read(EEPROM_CAL_VALID) != 0xAB) return false;
    EEPROM.get(EEPROM_CAL_X0, calX0);
    EEPROM.get(EEPROM_CAL_X1, calX1);
    EEPROM.get(EEPROM_CAL_Y0, calY0);
    EEPROM.get(EEPROM_CAL_Y1, calY1);
    calFlipX = EEPROM.read(EEPROM_CAL_FLIPX);
    calFlipY = EEPROM.read(EEPROM_CAL_FLIPY);
    return true;
}

void clearCalibration() {
    EEPROM.write(EEPROM_CAL_VALID, 0x00);
    EEPROM.commit();
}

// ---------------------------------------------------------------------------
// Raw → pixel coordinate mapping
// ---------------------------------------------------------------------------
static void rawToPixel(int16_t rx, int16_t ry, int16_t &px, int16_t &py) {
    int16_t x = map(rx, calX0, calX1, 0, SCREEN_W - 1);
    int16_t y = map(ry, calY0, calY1, 0, SCREEN_H - 1);
    if (calFlipX) x = (SCREEN_W - 1) - x;
    if (calFlipY) y = (SCREEN_H - 1) - y;
    px = constrain(x, 0, SCREEN_W - 1);
    py = constrain(y, 0, SCREEN_H - 1);
}

// ---------------------------------------------------------------------------
// Public touch read
// ---------------------------------------------------------------------------
bool getTouchPoint(int16_t &x, int16_t &y) {
    if (!ts.tirqTouched() || !ts.touched()) return false;
    TS_Point p = ts.getPoint();
    rawToPixel(p.x, p.y, x, y);
    return true;
}

bool isTouched() {
    return ts.tirqTouched() && ts.touched();
}

// ---------------------------------------------------------------------------
// Touch zone helpers — use constants from cyd_config.h
// ---------------------------------------------------------------------------
bool touchInZone(int16_t tx, int16_t ty, int16_t x1, int16_t y1,
                 int16_t x2, int16_t y2) {
    return (tx >= x1 && tx <= x2 && ty >= y1 && ty <= y2);
}

bool isUpZone(int16_t tx, int16_t ty) {
    return touchInZone(tx, ty, TOUCH_UP_X1, TOUCH_UP_Y1,
                               TOUCH_UP_X2, TOUCH_UP_Y2);
}
bool isDownZone(int16_t tx, int16_t ty) {
    return touchInZone(tx, ty, TOUCH_DOWN_X1, TOUCH_DOWN_Y1,
                               TOUCH_DOWN_X2, TOUCH_DOWN_Y2);
}
bool isBackZone(int16_t tx, int16_t ty) {
    return touchInZone(tx, ty, TOUCH_BACK_X1, TOUCH_BACK_Y1,
                               TOUCH_BACK_X2, TOUCH_BACK_Y2);
}
bool isSelectZone(int16_t tx, int16_t ty) {
    return touchInZone(tx, ty, TOUCH_SELECT_X1, TOUCH_SELECT_Y1,
                               TOUCH_SELECT_X2, TOUCH_SELECT_Y2);
}

// ---------------------------------------------------------------------------
// Screen rotation  (affects touch transform and TFT setRotation)
// ---------------------------------------------------------------------------
void setScreenRotation(uint8_t rot) {
    screenRotation = rot;
    ts.setRotation(rot);
    EEPROM.write(EEPROM_ROTATION, rot);
    EEPROM.commit();
}

uint8_t getScreenRotation() { return screenRotation; }

void loadRotation(TFT_eSPI &tft) {
    uint8_t r = EEPROM.read(EEPROM_ROTATION);
    if (r > 3) r = 0;
    screenRotation = r;
    tft.setRotation(r);
    ts.setRotation(r);
}

// ---------------------------------------------------------------------------
// Touch calibration routine (interactive 4-corner)
// Exactly matches original logic but adapted for 320x480.
// ---------------------------------------------------------------------------
void runTouchCalibration(TFT_eSPI &tft) {
    // 4 calibration points: TL, TR, BL, BR
    const int16_t pts[4][2] = {
        { 20,           20           },   // TL
        { SCREEN_W - 20, 20          },   // TR
        { 20,           SCREEN_H - 20},   // BL
        { SCREEN_W - 20, SCREEN_H - 20}   // BR
    };
    int32_t rawX[4], rawY[4];

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(20, SCREEN_H / 2 - 20);
    tft.println("Touch Calibration");
    tft.setCursor(20, SCREEN_H / 2 + 10);
    tft.println("Tap each crosshair");
    delay(1500);

    for (int i = 0; i < 4; i++) {
        int16_t cx = pts[i][0], cy = pts[i][1];

        // Draw crosshair
        tft.fillScreen(TFT_BLACK);
        tft.drawLine(cx - 15, cy,      cx + 15, cy,      TFT_RED);
        tft.drawLine(cx,      cy - 15, cx,      cy + 15, TFT_RED);
        tft.fillCircle(cx, cy, 3, TFT_RED);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(1);
        tft.setCursor(cx - 20, cy + 20);
        tft.print("Point ");
        tft.print(i + 1);

        // Wait for touch, take 5-sample average
        int32_t sx = 0, sy = 0, count = 0;
        while (count < 5) {
            if (ts.tirqTouched() && ts.touched()) {
                TS_Point p = ts.getPoint();
                sx += p.x; sy += p.y; count++;
                delay(50);
                while (ts.touched()) delay(10);   // debounce
            }
        }
        rawX[i] = sx / 5;
        rawY[i] = sy / 5;
        tft.fillCircle(cx, cy, 5, TFT_GREEN);
        delay(300);
    }

    // Derive calibration from corners
    // TL=0, TR=1, BL=2, BR=3
    calX0 = (rawX[0] + rawX[2]) / 2;   // left  raw
    calX1 = (rawX[1] + rawX[3]) / 2;   // right raw
    calY0 = (rawY[0] + rawY[1]) / 2;   // top   raw
    calY1 = (rawY[2] + rawY[3]) / 2;   // bottom raw

    // Detect axis flip (raw max < raw min means axis is inverted)
    calFlipX = (calX1 < calX0);
    calFlipY = (calY1 < calY0);
    if (calFlipX) { int16_t t = calX0; calX0 = calX1; calX1 = t; }
    if (calFlipY) { int16_t t = calY0; calY0 = calY1; calY1 = t; }

    saveCalibration();

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(20, SCREEN_H / 2 - 10);
    tft.println("Calibration saved!");
    delay(1500);
}
