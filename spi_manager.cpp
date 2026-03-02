// =============================================================================
// spi_manager.cpp  —  E32R40T port
// =============================================================================
// VSPI bus arbitration: SD card + CC1101 + NRF24
//
// E32R40T changes vs 2.8" CYD:
//   CC1101_CS_PIN : 21  (was 27 — 27 is now backlight)
//   NRF24_CSN_PIN : 32  (was 4  —  4  is now audio enable)
//   HSPI (display/touch) is managed separately — not touched here.
//
// Rule: Only ONE VSPI device active at a time.
//       Call selectDevice() before any SPI transaction.
//       Device is deselected by raising its CS after the transaction.
// =============================================================================

#include "spi_manager.h"
#include "cyd_config.h"
#include <SPI.h>

// VSPI instance — shared by SD, CC1101, NRF24
SPIClass vspi(VSPI);

void spiManagerInit() {
    vspi.begin(VSPI_SCK_PIN, VSPI_MISO_PIN, VSPI_MOSI_PIN);

    // Initialise all CS pins HIGH (deselected)
    pinMode(SD_CS_PIN,      OUTPUT); digitalWrite(SD_CS_PIN,      HIGH);
    pinMode(CC1101_CS_PIN,  OUTPUT); digitalWrite(CC1101_CS_PIN,  HIGH);
    pinMode(NRF24_CSN_PIN,  OUTPUT); digitalWrite(NRF24_CSN_PIN,  HIGH);
}

// Deselect all VSPI devices — call before selecting a specific one
void deSelectAll() {
    digitalWrite(SD_CS_PIN,      HIGH);
    digitalWrite(CC1101_CS_PIN,  HIGH);
    digitalWrite(NRF24_CSN_PIN,  HIGH);
}

void selectSD() {
    deSelectAll();
    digitalWrite(SD_CS_PIN, LOW);
}

void selectCC1101() {
    deSelectAll();
    digitalWrite(CC1101_CS_PIN, LOW);
}

void selectNRF24() {
    deSelectAll();
    digitalWrite(NRF24_CSN_PIN, LOW);
}

void deselectAll() {
    deSelectAll();
}

SPIClass& getVSPI() {
    return vspi;
}
