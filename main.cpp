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

#include "wirish.h"
#include "libraries/Card/SecureDigital/SDMC.h"

static const uint16 SIZE = 64;

SecureDigitalMemoryCard SDMC;

void setup() {
    pinMode(BOARD_LED_PIN, OUTPUT);
    digitalWrite(BOARD_LED_PIN, HIGH);
}

void loop() {
    waitForButtonPress();
    SerialUSB.println("SDIO_DBG: Starting SDMC test");
    SDMC.begin();
    uint8 myByteArray[SIZE];
    for (int i = 0; i < SIZE; i++) {
        myByteArray[i] = 0;
    }
    SDMC.getSSR((uint32*)&myByteArray);
    SDMC.end();
    for (int i = 0; i < SIZE; i++) {
        SerialUSB.print(myByteArray[i], HEX);
        if ((i+1)%4 == 0) {
            SerialUSB.println("");
        }
    }
    SerialUSB.println("SDIO_DBG: Test complete");
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