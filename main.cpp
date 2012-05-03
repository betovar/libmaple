/**
 * SDMC test will try to initialize a memory card and read it's registers.
 * 
 * Instructions: Plug any working micro SD card into the slot on the
 *               Maple Native board. Listen on SerialUSB, and press the 
 *               on-board button to begin the test.
 *
 * This file is released into the public domain.
 *
 * Author: Brian E Tovar <betovar@leaflabs.com>
 */

#define SDMC_SD_BUS_PROTOCOL // not currently being used yet
#include "wirish.h"
#include "libraries/Card/SecureDigital/SDMC.h"

SecureDigitalMemoryCard SDMC;

void setup() {
    pinMode(BOARD_LED_PIN, OUTPUT);
    SerialUSB.read();
    toggleLED();
    waitForButtonPress();
    SerialUSB.print("OCR: ");
    SerialUSB.println(sizeof(SDMC.OCR)*8, DEC);
    SerialUSB.print("SCR: ");
    SerialUSB.println(sizeof(SDMC.SCR)*8, DEC);
    SerialUSB.print("CID: ");
    SerialUSB.println(sizeof(SDMC.CID)*8, DEC);
    SerialUSB.print("CSD: ");
    SerialUSB.println(sizeof(SDMC.CSD)*8, DEC);

}

void loop() {
    waitForButtonPress();
    toggleLED();
    SerialUSB.println("*** Running SDMC test...");
    SDMC.begin();
    SerialUSB.println("*** Initializing card...");
    SDMC.init();
    SerialUSB.println("SDIO_DBG: finished init statement");
    SerialUSB.print("Serial Number: ");
    SerialUSB.println(SDMC.CID.PSN, DEC);
    SerialUSB.print("SD Version Number: ");
    switch (SDMC.CSD.version) {
    case 1:
        SerialUSB.println(SDMC.CSD.V1.CSD_STRUCTURE + 1, DEC);
        break;
    case 2:
        SerialUSB.println(SDMC.CSD.V2.CSD_STRUCTURE + 1, DEC);
        break;
    default:
        SerialUSB.println("Possible CMD8 error");
    }
    SDMC.end();
    toggleLED();
}

// Force init to be called *first*, i.e. before static object allocation.
// Otherwise, statically allocated objects that need libmaple may fail.
__attribute__((constructor)) void premain() {
    init();
}

int main(void) {
    setup();
    while (true) {
        loop();
    }
    return 0;
}
