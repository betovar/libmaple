/******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2012 LeafLabs LLC
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


#include "libmaple_types.h"
#include "sdio.h"
//#include "boards.h"

#ifndef _HARDWARESDIO_H_
#define _HARDWARESDIO_H_

/*
 * Types
 */

typedef enum SDIOPower {
    SDIO_POWER_OFF = 0,
    // Reserved    = 1,
    SDIO_POWER_UP  = 2,
    SDIO_POWER_ON  = 3,
} SDIOPowerState;

typedef enum SDIODataBusWidth {
    SDIO_DBW_0 = 0, // SDIO initialization default
    SDIO_DBW_4 = 1,
    SDIO_DBW_8 = 2, // MultiMedia Cards only
} SDIODataBusWidth;

typedef enum SDIODataBusMode {
    SDIO_DBM_DEFAULT = 0, // 25 MHz
    SDIO_DBM_HIGH, // 50 MHz
    // The folliwing bus speeds require 1.8V signaling
    SDIO_DBM_SDR12,
    SDIO_DBM_SDR25,
    SDIO_DBM_SDR50,
    SDIO_DBM_SDR104,
    SDIO_DBM_DDR50,
} SDIODataBusMode;

typedef enum SDIOFrequency {
    SDIO_36_MHZ  = 0,
    SDIO_25_MHZ  = 1,
    SDIO_12_MHZ  = 2,
    SDIO_6_MHZ   = 4,
    SDIO_3_MHZ   = 8,
    SDIO_1_MHZ   = 16,
    SDIO_500_KHZ = 32,
    SDIO_400_KHZ = 64,
    SDIO_200_KHZ = 128,
    SDIO_INIT_FREQ = 254,
} SDIOFrequency;

typedef enum SDIODataBlockSize {
    SDIO_DBSZ_1     = 0,
    SDIO_DBSZ_2     = 1,
    SDIO_DBSZ_4     = 2,
    SDIO_DBSZ_8     = 3,
    SDIO_DBSZ_16    = 4,
    SDIO_DBSZ_32    = 5,
    SDIO_DBSZ_64    = 6,
    SDIO_DBSZ_128   = 7,
    SDIO_DBSZ_256   = 8,
    SDIO_DBSZ_512   = 9,
    SDIO_DBSZ_1024  = 10,
    SDIO_DBSZ_2048  = 11,
    SDIO_DBSZ_4096  = 12,
    SDIO_DBSZ_8192  = 13,
    SDIO_DBSZ_16384 = 14,
} SDIODataBlockSize;

/**
 * @brief Wirish SDIO interface
 */
class HardwareSDIO {
public:
    /**
     * @brief SDIO class constructor
     * @param 
     */
    HardwareSDIO(void);

    /**
     * @brief 
     * @param freq
     * @param width 
     * @param mode
     */
    void begin(void);

    /**
     * @brief 
     * @param freq
     * @param width 
     * @param mode
     */
    void begin(SDIOFrequency freq, SDIODataBusWidth width, SDIODataBusMode mode);

    /**
     * @brief Disables the SDIO port, but leaves its GPIO pin modes unchanged.
     */
    void end(void);


/*
 * I/O
 */

    /**
     * @brief 
     * @param 
     */
    void read(uint8 *buffer, uint32 length);


    /**
     * @brief 
     * @param 
     */
    void write(const uint8 *buffer, uint32 length);

    uint32 command(uint8);
    void card_reset(void);
    void valid_voltage_range(void);
    void blockRead(void);
    void blockWrite(void);
    void card_identification(void);
    void card_status_register(void);
    void sd_status_register(void);
    void write_protect(void);
    void abort(void);
    void erase(void);


private:
    sdio_dev *sdio_d;
    void dmaConfig(void);
    void clockConfig(SDIOFrequency freq, SDIODataBusWidth width, SDIODataBusMode mode);
};

#endif

