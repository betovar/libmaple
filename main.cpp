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

#include "sdio.h"
#include "wirish.h"

void setup() {
    pinMode(BOARD_LED_PIN, OUTPUT);
    digitalWrite(BOARD_LED_PIN, HIGH);

    ASSERT(sdio_card_detect()); // also good for init gpios
    uint32 clockCR = (253 & SDIO_CLKCR_CLKDIV);
    sdio_set_clkcr(SDIO, clockCR);
    (SDIO->regs->CLKCR == clockCR);
    sdio_init(SDIO);

    sdio_power_off(SDIO);
    ASSERT(SDIO->regs->POWER == SDIO_POWER_OFF);
    sdio_power_on(SDIO);
    ASSERT(SDIO->regs->POWER == SDIO_POWER_ON);

    sdio_clock_disable(SDIO);
    ASSERT(SDIO->regs->CLKCR & ~SDIO_CLKCR_CLKEN);
    sdio_clock_enable(SDIO);
    ASSERT(SDIO->regs->CLKCR | SDIO_CLKCR_CLKEN);

    uint32 arg = 0xFF00F0F0;
    sdio_load_arg(SDIO, arg);
    ASSERT(SDIO->regs->ARG == arg);
}

void loop() {
    // LED blinks slowly for test complete without error
    togglePin(BOARD_LED_PIN);
    delay(1000);
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