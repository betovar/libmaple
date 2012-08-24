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
    SerialUSB.println("*** Starting SDMC test ***");
    SDMC.begin();
/*
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
    */
    SerialUSB.println("*** SDMC test complete ***");
    uint32 *cp = (uint32*)&SDMC.CID;
    for(int i=0; i<=3; i++) {
        SerialUSB.print(i);
        SerialUSB.print(": ");
        SerialUSB.println(cp[i], HEX);
    }
    cid test;
    uint32 *tp = (uint32*)&test;
    for(int i=0; i<=3; i++) {
        tp[i] = 0;
    }
    test.MID = 0xFF;
    test.OID[0] = 'O';
    test.OID[1] = 'K';
    for(int i=0; i<=3; i++) {
        SerialUSB.print(i);
        SerialUSB.print(": ");
        SerialUSB.println(tp[i], HEX);
    }
    icr test_icr;
    uint32 *tip = (uint32*)&test_icr;
    test_icr.CHECK_PATTERN = 0xAB;
    test_icr.VOLTAGE_ACCEPTED = 0x1;
    test_icr.Reserved1 = 0;
    SerialUSB.println(tip[0], HEX);
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