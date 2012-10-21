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
    //initialze cache block
    for (int i = 0; i < SIZEOF_CACHE; i++) {
        cacheBlock[i++] = 0x1E;
        cacheBlock[i++] = 0xAF;
        cacheBlock[i++] = 0x1A;
        cacheBlock[i++] = 0xB5; //0xLEAFLABS!
    }
}

void loop() {
    waitForButtonPress();
    SerialUSB.println("*** Starting SDMC test ***");
    SDMC.begin();
    SDMC.getCSD();
    SerialUSB.print("SDIO_DBG: RESP1 0x");
    SerialUSB.println(sdio_get_resp1(), HEX);
    SerialUSB.print("SDIO_DBG: RESP2 0x");
    SerialUSB.println(sdio_get_resp2(), HEX);
    SerialUSB.print("SDIO_DBG: RESP3 0x");
    SerialUSB.println(sdio_get_resp3(), HEX);
    SerialUSB.print("SDIO_DBG: RESP4 0x");
    SerialUSB.println(sdio_get_resp4(), HEX);
    SDMC.getCID();
    SerialUSB.print("SDIO_DBG: RESP1 0x");
    SerialUSB.println(sdio_get_resp1(), HEX);
    SerialUSB.print("SDIO_DBG: RESP2 0x");
    SerialUSB.println(sdio_get_resp2(), HEX);
    SerialUSB.print("SDIO_DBG: RESP3 0x");
    SerialUSB.println(sdio_get_resp3(), HEX);
    SerialUSB.print("SDIO_DBG: RESP4 0x");
    SerialUSB.println(sdio_get_resp4(), HEX);
  //SDMC.write(1000, (uint32*)cacheBlock, 1);
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