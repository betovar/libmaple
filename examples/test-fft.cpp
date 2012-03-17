/*
 * Simple Fast Fourier Transform test.
 *
 * Author: Brian E Tovar <betovar@leaflabs.com>
 *
 * This file is released into the public domain.
 */

#include "wirish.h"
#include "MapleFFT.h"

uint16 count = 0;

void setup() {
    pinMode(BOARD_LED_PIN, OUTPUT);
    digitalWrite(BOARD_LED_PIN, HIGH);

    Serial1.begin(9600);
    Serial1.println("**** Beginning FFT test");


}

void loop() {

}

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


