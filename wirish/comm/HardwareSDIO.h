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

#include "sdio.h"
#include "libmaple_types.h"

#ifndef _HARDWARESDIO_H_
#define _HARDWARESDIO_H_

/*
 * Types
 */

typedef enum SDIOPowerState {
    SDIO_PWR_OFF = 0,
    // Reserved  = 1,
    SDIO_PWR_UP  = 2,
    SDIO_PWR_ON  = 3
} SDIOPowerState;

typedef enum SDIOWaitResp {
    SDIO_WRSP_NONE     = 0,
    SDIO_WRSP_SHRT     = 1,
    //SDIO_WRSP_NONE_2 = 2,
    SDIO_WRSP_LONG     = 3
} SDIOWaitResp;

typedef enum SDIODataBusWidth {
    SDIO_DBW_1 = 0, // SDIO initialization default
    SDIO_DBW_4 = 1,
    SDIO_DBW_8 = 2  // MultiMedia Cards only
} SDIODataBusWidth;

typedef enum SDIOFrequency {
    SDIO_36_MHZ  = 0, // not supported for SD cards
    SDIO_24_MHZ  = 1,
    SDIO_18_MHZ  = 2,
    SDIO_12_MHZ  = 4,
    SDIO_6_MHZ   = 10,
    SDIO_3_MHZ   = 22,
    SDIO_2_MHZ   = 34,
    SDIO_1_MHZ   = 70,
    SDIO_500_KHZ = 142,
    SDIO_400_KHZ = 178,
    SDIO_300_KHZ = 238,
    SDIO_INIT_FREQ = 254 // 281,250 Hz
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
    SDIO_DBSZ_16384 = 14
} SDIODataBlockSize;

typedef enum SDIOInterruptFlag {
    SDIO_FLAG_CCRCFAIL = 0,
    SDIO_FLAG_DCRCFAIL = 1,
    SDIO_FLAG_CTIMEOUT = 2,
    SDIO_FLAG_DTIMEOUT = 3,
    SDIO_FLAG_TXUNDERR = 4,
    SDIO_FLAG_RXOVERR  = 5,
    SDIO_FLAG_CMDREND  = 6,
    SDIO_FLAG_CMDSENT  = 7,
    SDIO_FLAG_DATAEND  = 8,
    SDIO_FLAG_STBITERR = 9,
    SDIO_FLAG_DBCKEND  = 10,
    SDIO_FLAG_CMDACT   = 11,
    SDIO_FLAG_TXACT    = 12,
    SDIO_FLAG_RXACT    = 13,
    SDIO_FLAG_TXFIFOHE = 14,
    SDIO_FLAG_RXFIFOHF = 15,
    SDIO_FLAG_TXFIFOF  = 16,
    SDIO_FLAG_RXFIFOF  = 17,
    SDIO_FLAG_TXFIFOE  = 18,
    SDIO_FLAG_RXFIFOE  = 19,
    SDIO_FLAG_TXDAVL   = 20,
    SDIO_FLAG_RXDAVL   = 21,
    SDIO_FLAG_SDIOIT   = 22,
    SDIO_FLAG_CEATAEND = 23
} SDIOInterruptFlag;
/**
 * @brief Wirish SDIO interface
 */
class HardwareSDIO {
  public:
    HardwareSDIO(void);

    void begin(void);
    void begin(SDIOFrequency, SDIODataBusWidth);
    void end(void);

    uint32 read(void);
  //read(uint8*, uint32)
    void write(uint32);
  //write(const uint8*, uint32)
    void send(uint8);
    void send(uint8, uint32);
    void send(uint8, uint32, void*);

  protected:
    sdio_dev *sdio_d;

    void power(SDIOPowerState);
    void freq(SDIOFrequency);
    void bus(SDIODataBusWidth);
    //void dmaConfig(void);
    
};

#endif