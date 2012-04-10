/**
 * SDMC test will try to initialize a memory card and read it's registers.
 * 
 * Instructions: Plug a micro SD card into the slot on a Maple Native board.
 *
 * This file is released into the public domain.
 *
 * Author: Brian E Tovar <betovar@leaflabs.com>
 */

#include "wirish.h"
#include "libraries/Card/SecureDigital/SecureDigitalMemoryCard.h"

SecureDigitalMemoryCard SDMC;

void setup() {
    pinMode(BOARD_LED_PIN, OUTPUT);
    SerialUSB.read();
}

void loop() {
    waitForButtonPress();

     SerialUSB.println("*** Running SDMC test...");
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
