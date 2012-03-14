/******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2010 Perry Hung.
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
 * @file HardwareSDIO.h
 * @author Brian E Tovar <betovar@leaflabs.com>
 * @brief SDIO class
 */

/* TODO [0.1.0] Remove deprecated methods. */

#include "libmaple_types.h"
#include "sdio.h"

#include "boards.h"

#ifndef _HARDWARESDIO_H_
#define _HARDWARESDIO_H_

/*
 * Types
 */

typedef enum SDIODataBusWidth {
    SDIO_DBW_0 = 0,
    SDIO_DBW_4 = 4,
    SDIO_DBW_8 = 8
} SDIODataBusWidth;

typedef enum SDIODataBusSpeed {
    SDIO_DBS_DEFAULT = 0, // 25 MHz
    SDIO_DBS_HIGH, // 50 MHz
    // The folliwing bus speeds require 1.8V signaling
    SDIO_DBS_SDR12,
    SDIO_DBS_SDR25,
    SDIO_DBS_SDR50,
    SDIO_DBS_SDR104,
    SDIO_DBS_DDR50
} SDIODataBusSpeed;

#define MAX_SPI_FREQS 8

#if CYCLES_PER_MICROSECOND != 72
/* TODO [0.2.0?] something smarter than this */
#warning "Unexpected clock speed; SPI frequency calculation will be incorrect"
#endif

/**
 * @brief Wirish SPI interface.
 *
 * This implementation uses software slave management, so the caller
 * is responsible for controlling the slave select line.
 */
class HardwareSDIO {
public:
    /**
     * @param spiPortNumber Number of the SPI port to manage.
     */
    HardwareSPI(uint32 spiPortNumber);


    void begin(SPIFrequency frequency, uint32 bitOrder, uint32 mode);

    /**
     * @brief Equivalent to begin(SPI_1_125MHZ, MSBFIRST, 0).
     */
    void begin(void);

    /**
     * @brief Disables the SPI port, but leaves its GPIO pin modes unchanged.
     */
    void end(void);

    /*
     * I/O
     */


    /**
     * @brief Read length bytes, storing them into buffer.
     * @param buffer Buffer to store received bytes into.
     * @param length Number of bytes to store in buffer.  This
     *               function will block until the desired number of
     *               bytes have been read.
     */
    void read(uint8 *buffer, uint32 length);


    /**
     * @brief Transmit multiple bytes.
     * @param buffer Bytes to transmit.
     * @param length Number of bytes in buffer to transmit.
     */
    void write(const uint8 *buffer, uint32 length);

void sdio_gpio_init_cfg(SDIODataBusWidth data_bus_width);
void sdio_card_id_process(void);			
void sdio_configure_dma(void);
void sdio_send_cmd(void);

private:
    spi_dev *spi_d;
};

#endif

