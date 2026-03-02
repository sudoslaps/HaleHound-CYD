// spi_manager.h  —  E32R40T port
#pragma once
#include <SPI.h>

extern SPIClass vspi;

void      spiManagerInit();
void      deSelectAll();
void      selectSD();
void      selectCC1101();
void      selectNRF24();
void      deselectAll();
SPIClass& getVSPI();
