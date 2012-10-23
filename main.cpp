/**
 * SDMC test will try to initialize a memory card and read it's registers.
 * 
 * Instructions: Plug any working micro SD card into the slot on the
 *               Maple Native board. Listen on SerialUSB, and press the 
 *               on-board button to begin the test.
 *
 * This file is released into the public domain.
 *  
 * Author: Brian E Tovar
 * Email:  <betovar@leaflabs.com>
 */

#include "wirish.h"
#include "libraries/Card/SecureDigital/SDMC.h"

#define SIZEOF_CACHE 512

HardwareSDIO SDMC;
uint8 cacheBlock[SIZEOF_CACHE];

void setup() {
    pinMode(BOARD_LED_PIN, OUTPUT);
    digitalWrite(BOARD_LED_PIN, HIGH);
}

void loop() {
    waitForButtonPress();
    SerialUSB.println("*** Starting SDMC test ***");
    SDMC.begin();
    SDMC.getCSD();
    SerialUSB.print("CSD: RESP1 0x");
    SerialUSB.println(sdio_get_resp1(), HEX);
    SerialUSB.print("CSD: RESP2 0x");
    SerialUSB.println(sdio_get_resp2(), HEX);
    SerialUSB.print("CSD: RESP3 0x");
    SerialUSB.println(sdio_get_resp3(), HEX);
    SerialUSB.print("CSD: RESP4 0x");
    SerialUSB.println(sdio_get_resp4(), HEX);
    SDMC.getCID();
    SerialUSB.print("CID: RESP1 0x");
    SerialUSB.println(sdio_get_resp1(), HEX);
    SerialUSB.print("CID: RESP2 0x");
    SerialUSB.println(sdio_get_resp2(), HEX);
    SerialUSB.print("CID: RESP3 0x");
    SerialUSB.println(sdio_get_resp3(), HEX);
    SerialUSB.print("CID: RESP4 0x");
    SerialUSB.println(sdio_get_resp4(), HEX);
    SDMC.end();
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