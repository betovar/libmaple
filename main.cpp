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
    //SDMC.begin();
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

    icr test_icr;
    test_icr.CHECK_PATTERN = 0;
    test_icr.VOLTAGE_ACCEPTED = 0;
    test_icr.Reserved1 = 0;

    uint32 *tip = (uint32*)&test_icr;
    SerialUSB.print("testing icr: ");
    SerialUSB.println(tip[0], HEX);
    test_icr.CHECK_PATTERN = 0xAA;
    test_icr.VOLTAGE_ACCEPTED = 0x1;
    test_icr.Reserved1 = 0;
    SerialUSB.print("             ");
    SerialUSB.println(tip[0], HEX);

    cid test_cid;
    /**
    test_cid.MID = 0;
    test_cid.OID[1] = 0;
    test_cid.OID[0] = 0;
    for(int i=0; i<=4; i++) {
        test_cid.PNM[i] = 0;
    }
    test_cid.PRV.N = 0;
    test_cid.PRV.M = 0;
    test_cid.PSN = 0;
    test_cid.Reserved1 = 0;
    test_cid.MDT.YEAR = 0;
    test_cid.MDT.MONTH = 0;
    test_cid.CRC = 0;
    test_cid.Always1 = 0;
    test_cid.Reserved1 = 0;
    */
    
    uint32 *tcp = (uint32*)&test_cid;
    SerialUSB.println("Initializing test_cid: ");
    for(int i=0; i<=3; i++) {
        tcp[i] = 0;
        SerialUSB.print(i+1);
        SerialUSB.print(": ");
        SerialUSB.println(tcp[i], HEX);
    }

    test_cid.MID = 0xaA;
    test_cid.OID[0] = 0xBB;
    test_cid.OID[1] = 0xCC;
    for(int i=0; i<=4; i++) {
        test_cid.PNM[i] = ((i+1)<<8)+(i+1);
    }
    test_cid.PRV.N = 0xD;
    test_cid.PRV.M = 0xD;
    test_cid.PSN = 0xDEADBEEF;
    test_cid.MDT.YEAR = 0xEE;
    test_cid.MDT.MONTH = 0x6;
    SerialUSB.print("Size of CID: ");
    SerialUSB.println(sizeof(test_cid), DEC);
    SerialUSB.println("Testing cid: ");
    for(int i=0; i<=3; i++) {
        SerialUSB.print(i+1);
        SerialUSB.print(": ");
        SerialUSB.println(tcp[i], HEX);
    }

    /**
    csd test_csd;
    uint32 *tsp = (uint32*)&test_csd;
    for(int i=0; i<=3; i++) {
        tsp[i] = 0;
    }
    test_csd.CSD_STRUCTURE = 0xFF;
    SerialUSB.println("testing csd: ");
    for(int i=0; i<=3; i++) {
        SerialUSB.print(i+1);
        SerialUSB.print(": ");
        SerialUSB.println(tsp[i], HEX);
    }
    */

    SerialUSB.println();

    csr test_csr;
    test_csr.OUT_OF_RANGE       = 0;
    test_csr.ADDRESS_ERROR      = 0;
    test_csr.BLOCK_LEN_ERROR    = 0;
    test_csr.ERASE_SEQ_ERROR    = 0;
    test_csr.ERASE_PARAM        = 0;
    test_csr.WP_VIOLATION       = 0;
    test_csr.CARD_IS_LOCKED     = 0;
    test_csr.LOCK_UNLOCK_FAILED = 0;
    test_csr.COM_CRC_ERROR      = 0;
    test_csr.ILLEGAL_COMMAND    = 0;
    test_csr.CARD_ECC_FAILED    = 0;
    test_csr.CC_ERROR           = 0;
    test_csr.ERROR              = 0;
    test_csr.CSD_OVERWRITE      = 0;
    test_csr.WP_ERASE_SKIP      = 0;
    test_csr.CARD_ECC_DISABLED  = 0;
    test_csr.ERASE_RESET        = 0;
    test_csr.CURRENT_STATE      = 0xF;
    test_csr.READY_FOR_DATA     = 0;
    test_csr.APP_CMD            = 0;
    test_csr.AKE_SEQ_ERROR      = 0;
    uint32 *trp = (uint32*)&test_csr;
    SerialUSB.println("Initializing test_csr: ");
    SerialUSB.println(*(trp), HEX);
    SerialUSB.print("Size of CSR: ");
    SerialUSB.println(sizeof(test_csr), DEC);
    SerialUSB.println("Testing r1: ");
    SerialUSB.println(*(trp), HEX);
    SerialUSB.println(test_csr.CURRENT_STATE, HEX);
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