/**
 * SDMC test will try to initialize a memory card and read it's registers.
 * 
 * Instructions: Plug a micro SD card into the slot on a Maple Native board.
 *
 * This file is released into the public domain.
 *
 * Author: Brian E Tovar <betovar@leaflabs.com>
 */

#define SD_SD_BUS_PROTOCOL
#include "wirish.h"
#include "libraries/Card/SecureDigital/SDMC.h"

SecureDigitalMemoryCard SDMC;

void setup() {
    pinMode(BOARD_LED_PIN, OUTPUT);
    SerialUSB.read();
}

void loop() {
    waitForButtonPress();
    toggleLED();
    SerialUSB.println("*** Running SDMC test...");
    SDMC.begin();
    SDMC.init();
    SerialUSB.print("Serial Number: ");
    SerialUSB.println(SDMC.CID.PSN, DEC);
    SDMC.end();
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
