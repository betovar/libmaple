/******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2012 LeafLabs, LLC
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *****************************************************************************/

/**
 * @file SecureDigitalMemoryCard.cpp
 * @author Brian E Tovar <betovar@leaflabs.com>
 * @brief Wirish SD card implementation using HardwareSDIO. 
 */

#include "SecureDigitalMemoryCard.h"

SecureDigitalMemoryCard::SecureDigitalMemoryCard() {
}

 /*
 * I/O
 */

/*
void SecureDigitalMemoryCard::read(uint8 *buf, uint32 len) {
    uint32 rxed = 0;
    while (rxed < len) {
        while (!spi_is_rx_nonempty(this->spi_d))
            buf[rxed++] = (uint8)spi_rx_reg(this->spi_d);
    }
}

void SecureDigitalMemoryCard::write(const uint8 *data, uint32 length) {
    uint32 txed = 0;
    while (txed < length) {
        txed += spi_tx(this->spi_d, data + txed, length - txed);
    }
}
*/

 /*
 * Setup functions
 */

/**
 * @brief Set bus width in clock control register
 * @param width WIDBUS value to set
 */
void SecureDigitalMemoryCard::busWidth(uint8 width) {
    /* WIDBUS: width of data bus is set */
    if (width <= 1) {
        sdio_cfg_cpsm(this->sdio_d, SDIO_CLKCR_WIDBUS, 
                      (width << SDIO_CLKCR_WIDBUS_BIT) );
    } else {
        ASSERT(0); //TODO[0.2.0] add support for UHS cards
    }
    //FIXME
    // send command to set bus width in card
    //this->command((SD)ACMD6 or (SDIO)CMD52);
}

/*
 * Card specific functions
 */

/**
 * @brief Load the 
 */
void SecureDigitalMemoryCard::getCID(void) {

}

/**
 * @brief Load the Card Specific Data into memory 
 */
void SecureDigitalMemoryCard::getCSD(void) {

}

/**
 * @brief Load the 
 */
void SecureDigitalMemoryCard::getCSR(void) {

}

/**
 * @brief Load the 
 */
void SecureDigitalMemoryCard::getSSR(void) {

}

/*
 * Card convenience functions
 */

/**
 * @brief Configure GPIO bit modes for use as an SDIO port's pins
 * @param data_bus_width Enum to configure pins for use as an SDIO card
 * @note 8-bit data bus width not implemented on maple as of March 2012
 */
void SecureDigitalMemoryCard::card_identification_process(void) {
    //activate bus
    //host broadcasts SD_APP_OP_COND
    //card resp: ocr
    //place incompatible cards to inactive state
    //host broadcasts ALL_SEND_COND 
    //card resp: cid numbers
    //host issues: SET_RELATIVE_ADDR (rca)
    //
}