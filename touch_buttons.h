// touch_buttons.h  —  E32R40T port
#pragma once
#include <TFT_eSPI.h>
#include <stdint.h>

// Touch init / read
void    touchInit();
bool    getTouchPoint(int16_t &x, int16_t &y);
bool    isTouched();
bool    touchInZone(int16_t tx, int16_t ty, int16_t x1, int16_t y1,
                    int16_t x2, int16_t y2);

// Named zone helpers (coordinates from cyd_config.h)
bool    isUpZone(int16_t tx, int16_t ty);
bool    isDownZone(int16_t tx, int16_t ty);
bool    isBackZone(int16_t tx, int16_t ty);
bool    isSelectZone(int16_t tx, int16_t ty);

// Calibration
bool    loadCalibration();
void    clearCalibration();
void    runTouchCalibration(TFT_eSPI &tft);

// Backlight
void    backlightInit();
void    setBrightness(uint8_t val);
uint8_t getBrightness();
void    saveBrightness();
void    loadBrightness();

// Rotation
void    setScreenRotation(uint8_t rot);
uint8_t getScreenRotation();
void    loadRotation(TFT_eSPI &tft);
