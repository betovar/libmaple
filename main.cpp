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

HardwareSDIO SDMC;

void setup() {
    pinMode(BOARD_LED_PIN, OUTPUT);
    digitalWrite(BOARD_LED_PIN, HIGH);
}

void loop() {
    waitForButtonPress();
    SerialUSB.println("*** Starting SDMC test ***");
    SDMC.begin();
    //SDMC.readBlock(1000, (uint32*)this->cache[0]);
    SDMC.getSSR();
    SDMC.end();
    /*
    for (int i = 0; i < 512; i++) {
        SerialUSB.print(SDMC.cache[i], HEX);
        SerialUSB.print(" ");
        if ((i+1)%4 == 0) {
            SerialUSB.println("");
        }
    }
    */
    SerialUSB.println("*** SDMC test complete ***");
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