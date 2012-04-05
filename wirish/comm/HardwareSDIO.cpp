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
 * @file HardwareSDIO.cpp
 * @author Brian E Tovar <betovar@leaflabs.com>
 * @brief Wirish SDIO implementation. 
 */

#include "HardwareSDIO.h"

#include "sdio.h"
#include "util.h"
#include "rcc.h"

#include "wirish.h"
//#include "boards.h"

#if CYCLES_PER_MICROSECOND != 72
/* TODO [0.2.0?] something smarter than this */
#warning "Unexpected clock speed; SDIO frequency calculation will be incorrect"
#endif

/*
 * Constructor
 */

HardwareSDIO::HardwareSDIO(void) {
    this->sdio_d = SDIO;
}

/*
 * Public Members
 */

/**
 * @brief 
 * @param 
 */
void HardwareSDIO::begin(void) {
    this->begin(SDIO_INIT_FREQ, SDIO_DBW_0);
}

/**
 * @brief 
 * @param freq
 * @param width
 * @param mode
 */
void HardwareSDIO::begin(SDIOFrequency freq,
                         SDIODataBusWidth width) {
    sdio_cfg_clock(this->sdio_d, (uint8)freq);
    sdio_cfg_bus(this->sdio_d, (uint8)width);
//    sdio_set_ccr(this->sdio_d, SDIO_CLKCR_CLKEN, (0x1 << SDIO_CLKCR_CLKEN_BIT));
    sdio_peripheral_enable(this->sdio_d);
}

void HardwareSDIO::end(void) {
    sdio_reset(this->sdio_d);
}

/*
 * I/O
 */

/*
void HardwareSDIO::read(uint8 *buf, uint32 len) {
    uint32 rxed = 0;
    while (rxed < len) {
        while (!spi_is_rx_nonempty(this->spi_d))
            buf[rxed++] = (uint8)spi_rx_reg(this->spi_d);
    }
}

void HardwareSDIO::write(const uint8 *data, uint32 length) {
    uint32 txed = 0;
    while (txed < length) {
        txed += spi_tx(this->spi_d, data + txed, length - txed);
    }
}
*/

/*
 * Card specific functions
 */


/**
 * @brief Configure GPIO bit modes for use as an SDIO port's pins
 * @param data_bus_width Enum to configure pins for use as an SDIO card
 * @note 8-bit data bus width not implemented on maple as of March 2012
 */
void HardwareSDIO::card_identification_process(void) {
    //activate bus
    //host broadcasts SD_APP_OP_COND
    //card resp: ocr
    //place incompatible cards to inactive state
    //host broadcasts ALL_SEND_COND 
    //card resp: cid numbers
    //host issues: SET_RELATIVE_ADDR (rca)
    //
}

/**
 * @brief Set bus width in clock control register
 * @param width WIDBUS value to set
 */
void HardwareSDIO::busWidth(uint8 width) {
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