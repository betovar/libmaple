/*
 * Simple Fast Fourier Transform test.
 *
 * Author: Brian E Tovar <betovar@leaflabs.com>
 *
 * This file is released into the public domain.
 */

#include "wirish.h"
#include "MapleFFT.h"

//Number of points 
#define N 64

//Input and Output arrays
uint32 x[N],y[N];

//Real and Imaginary arrays
uint16 real[N], imag[N];

void setup() {
    Serial1.begin(9600);
    Serial1.println("**** Beginning FFT test");
}

void loop() {
    for(i=0; i<N; i++) {
        x[i] = (((uint16)(real[i])) | ((uint32)(imag[i]<<16))); 
    }
    //Computes the FFT of the x[N] samples
    cr4_fft_64_stm32(y, x, N);
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


