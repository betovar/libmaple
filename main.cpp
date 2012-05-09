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
    digitalWrite(BOARD_LED_PIN, HIGH);
}

void loop() {
    waitForButtonPress();
    SDMC.test();
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

/**
    toggleLED();
    SerialUSB.println("SDIO_DBG: Running SDMC test");
    SDMC.begin();
    SerialUSB.println("SDIO_DBG: Initializing card");
    SDMC.init();
    SerialUSB.println("SDIO_DBG: Test complete");
    SDMC.end();
    toggleLED();
*/