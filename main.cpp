/**
 * SDMC test will try to initialize a memory card and read it's registers.
 * 
 * Instructions: Use JTAG debugger
 *
 * This file is released into the public domain.
 *  
 * Author: Brian E Tovar
 * Email:  <betovar@leaflabs.com>
 */

#include <wirish/wirish.h>
#include <Card/SecureDigital/HardwareSDIO.h>

//#define SIZEOF_CACHE 512

HardwareSDIO SDMC;
//uint8 cacheBlock[SIZEOF_CACHE];

void setup() {
    enableDebugPorts(); // for debugging over JTAG
    pinMode(BOARD_LED_PIN, OUTPUT);
    digitalWrite(BOARD_LED_PIN, HIGH);
}

void loop() {
    //waitForButtonPress();
    SDMC.begin();
    SDMC.getCID();
    SDMC.getCSD();
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