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
#include "wirish.h"
#include "util.h"
#include "rcc.h"

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
 */
void HardwareSDIO::begin(void) {
    this->freq(SDIO_INIT_FREQ);
    this->bus(SDIO_DBW_1);
    uint32 widbus = ((uint32)width << SDIO_CLKCR_WIDBUS_BIT);
    sdio_cfg_clkcr(this->sdio_d, SDIO_CLKCR_WIDBUS, widbus);
    sdio_peripheral_enable(this->sdio_d);
}

void HardwareSDIO::end(void) {
    sdio_reset(this->sdio_d);
}

void HardwareSDIO::power(SDIOPowerState pwr) {
    switch (pwr) {
    case SDIO_PWR_ON:
        sdio_power(this->sdio_d, SDIO_POWER_ON);
        break;
    case SDIO_PWR_OFF:
        sdio_power(this->sdio_d, SDIO_POWER_OFF);
        break;
    default:
        ASSERT(0);
    }//end of switch
    //block-until/wait-for power on
    uint32 timeout = 1000;
    while (this->sdio_d->regs->POWER == SDIO_POWER_ON) {
        if (!timeout) {
            timeout--;
        } else {
            break;
        }
    };
}

/**
 * @brief Command (without response nor argument) to send to card
 * @param idx Command to send
 */
void HardwareSDIO::send(uint8 idx) {
    this->send(idx, 0, NULL);
}

/**
 * @brief Command (without response) to send to card
 * @param idx Command to send
 * @param arg Argument to send to card
 */
void HardwareSDIO::send(uint8 idx, uint32 arg) {
    this->send(idx, arg, NULL);
}

/**
 * @brief Command (with response) to send to card
 * @param idx Command Index to send
 * @param arg Argument to send
 * @param rsp Buffer to store response
 */
void HardwareSDIO::send(uint8 idx,
                        uint32 arg,
                        void *resp) {
    sdio_load_arg(this->sdio_d, arg);
    sdio_send_cmd(this->sdio_d, idx);
    sdio_get_resp_long(this->sdio_d, (uint32*)resp);
}

/**
 * @brief Read next word from FIFO 
 * @retval Data that was read from FIFO
 */
uint32 HardwareSDIO::read(void) {
    return sdio_read_data(this->sdio_d);
/**
    uint32 rxed = 0;
    while (rxed < len) {
        while (!spi_is_rx_nonempty(this->spi_d))
            buf[rxed++] = (uint8)spi_rx_reg(this->spi_d);
    }
    */
}

/**
 * @brief Write next word into FIFO
 * @param word Data to write to FIFO
 */
void HardwareSDIO::write(const uint32 word) {
    sdio_write_data(this->sdio_d, word);
/**
    uint32 txed = 0;
    while (txed < length) {
        txed += spi_tx(this->spi_d, data + txed, length - txed);
    }
    */
}