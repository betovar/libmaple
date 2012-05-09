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
 * @file SDMC.h
 * @author Brian E Tovar <betovar@leaflabs.com>
 * @brief Wirish SD Memory Card implementation
 */

#include "sdio.h"
#include "Structures.h"
#include "Commands.h"
#include "libmaple_types.h"

#ifndef _SDMC_H_
#define _SDMC_H_

/**
 * SDIO Enumerations
 */

typedef enum SDIOWaitResp {
    SDIO_WRSP_NONE     = 0,
    SDIO_WRSP_SHRT     = 1,
  //SDIO_WRSP_NONE_2   = 2,
    SDIO_WRSP_LONG     = 3
} SDIOWaitResp;

typedef enum SDIOBusMode {
    SDIO_BUS_INIT     = SDIO_GPIO_INIT,
    SDIO_BUS_RESPIN   = SDIO_GPIO_CMD_INPUT,
    SDIO_BUS_CMDOUT   = SDIO_GPIO_CMD_OUTPUT,
    SDIO_BUS_DAT1_IN  = SDIO_GPIO_1B_DATA_INPUT,
    SDIO_BUS_DAT1_OUT = SDIO_GPIO_1B_DATA_OUTPUT,
    SDIO_BUS_DAT4_IN  = SDIO_GPIO_4B_DATA_INPUT,
    SDIO_BUS_DAT4_OUT = SDIO_GPIO_4B_DATA_OUTPUT
  //SDIO_BUS_DAT8_IN  = SDIO_GPIO_8B_DATA_INPUT,
  //SDIO_BUS_DAT8_OUT = SDIO_GPIO_8B_DATA_OUTPUT
} SDIOBusMode;

typedef enum SDIOClockFrequency {
  //SDIO_36_MHZ   = 0, // not supported for SD cards
    SDIO_24_MHZ   = 1,
    SDIO_18_MHZ   = 2,
    SDIO_12_MHZ   = 4,
    SDIO_6_MHZ    = 10,
    SDIO_3_MHZ    = 22,
    SDIO_2_MHZ    = 34,
    SDIO_1_MHZ    = 70,
    SDIO_500_KHZ  = 142,
    SDIO_400_KHZ  = 178,
    SDIO_300_KHZ  = 238,
    SDIO_CLK_INIT = SDIO_400_KHZ
} SDIOClockFrequency;

typedef enum SDIOBlockSize {
    SDIO_BKSZ_1     = 0,
    SDIO_BKSZ_2     = 1,
    SDIO_BKSZ_4     = 2,
    SDIO_BKSZ_8     = 3,
    SDIO_BKSZ_16    = 4,
    SDIO_BKSZ_32    = 5,
    SDIO_BKSZ_64    = 6,
    SDIO_BKSZ_128   = 7,
    SDIO_BKSZ_256   = 8,
    SDIO_BKSZ_512   = 9,
    SDIO_BKSZ_1024  = 10,
    SDIO_BKSZ_2048  = 11,
    SDIO_BKSZ_4096  = 12,
    SDIO_BKSZ_8192  = 13,
    SDIO_BKSZ_16384 = 14
} SDIOBlockSize;

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
    //Convenience Flags
  //SDIO_FLAG_DYNAMIC  = 33,
  //SDIO_FLAG_STATIC   = 34,
  //SDIO_FLAG_COMMAND  = 35,
  //SDIO_FLAG_DATA     = 36,
  //SDIO_FLAG_ALL      = 37
} SDIOInterruptFlag;

typedef enum SDIOStatusResponseTag {
    //CardStatusResponse Tags
    SDIO_CSR_NO_ERROR        = 0,
    SDIO_CSR_ERROR           = 1,
    SDIO_CSR_NOT_PROTECTED   = 0,
    SDIO_CSR_PROTECTED       = 1,
    SDIO_CSR_CARD_UNLOCKED   = 0,
    SDIO_CSR_CARD_LOCKED     = 1,
    SDIO_CSR_SUCCESS         = 0,
    SDIO_CSR_FAILURE         = 1,
    SDIO_CSR_ECC_ENABLED     = 0,
    SDIO_CSR_ECC_DISABLED    = 1,
    SDIO_CSR_CLEARED         = 0,
    SDIO_CSR_SET             = 1,
    SDIO_CSR_IDLE            = 0,
    SDIO_CSR_NOT_READY       = 0,
    SDIO_CSR_READY           = 1,
    SDIO_CSR_IDENT           = 2,
    SDIO_CSR_STBY            = 3,
    SDIO_CSR_TRAN            = 4,
    SDIO_CSR_DATA            = 5,
    SDIO_CSR_RCV             = 6,
    SDIO_CSR_PRG             = 7,
    SDIO_CSR_DIS             = 8,
    SDIO_CSR_IO_MODE         = 15,
    SDIO_CSR_APP_DISABLED    = 0,
    SDIO_CSR_APP_ENABLED     = 1,
    //SdStatusResponse Tags
    SDIO_SSR_1BIT_WIDTH      = 0,
    SDIO_SSR_4BIT_WIDTH      = 2,
    SDIO_SSR_NOT_SECURED     = 0,
    SDIO_SSR_SECURED         = 1,
    SDIO_SSR_REG_CARD        = 0,
    SDIO_SSR_ROM_CARD        = 1,
    SDIO_SSR_OTP_CARD        = 2,
    SDIO_SSR_SPEED_CLASS_0   = 0,
    SDIO_SSR_SPEED_CLASS_2   = 1,
    SDIO_SSR_SPEED_CLASS_4   = 2,
    SDIO_SSR_SPEED_CLASS_6   = 3,
    SDIO_SSR_SPEED_CLASS_10  = 4,
    SDIO_SSR_AU_SIZE_NOT_DEF = 0,
    SDIO_SSR_AU_SIZE_16KB    = 1,
    SDIO_SSR_AU_SIZE_32KB    = 2,
    SDIO_SSR_AU_SIZE_64KB    = 3,
    SDIO_SSR_AU_SIZE_128KB   = 4,
    SDIO_SSR_AU_SIZE_256KB   = 5,
    SDIO_SSR_AU_SIZE_512KB   = 6,
    SDIO_SSR_AU_SIZE_1MB     = 7,
    SDIO_SSR_AU_SIZE_2MB     = 8,
    SDIO_SSR_AU_SIZE_4MB     = 9,
    SDIO_SSR_AU_SIZE_8MB     = 10,
    SDIO_SSR_AU_SIZE_12MB    = 11,
    SDIO_SSR_AU_SIZE_16MB    = 12,
    SDIO_SSR_AU_SIZE_24MB    = 13,
    SDIO_SSR_AU_SIZE_32MB    = 14,
    SDIO_SSR_AU_SIZE_64MB    = 15,
} SDIOStatusResponseTag;

class SecureDigitalMemoryCard {
  public:
    ocr OCR;
    scr SCR;
    cid CID;
    csd CSD;
    rca RCA;
    dsr DSR; // Default is 0x0404

    SecureDigitalMemoryCard();
    // startup.. functions
    void begin(void);
    void init(void);
    void test(void);
    void end(void);
    // convenience functions
    void clockFreq(SDIOClockFrequency);
    void busMode(SDIOBusMode);
    void blockSize(SDIOBlockSize);
    // command functions
    void cmd(SDIOCommand);
    void cmd(SDIOCommand, uint32);
    void cmd(SDIOCommand, uint32, SDIOWaitResp, uint32*);
    void acmd(SDIOAppCommand);
    void acmd(SDIOAppCommand, uint32);
    void acmd(SDIOAppCommand, uint32, SDIOWaitResp, uint32*);
    // general data functions
    void stop(void);
    void read(uint32, uint32*, uint32);
    void write(uint32, const uint32*, uint32);

  private:
    sdio_dev *sdio_d;
    // basic data functions
    void readBlock(uint32, uint32*);
    void writeBlock(uint32, const uint32*);
    // interrupt functions
    void enable(SDIOInterruptFlag);
    void disable(SDIOInterruptFlag);
    void clear(SDIOInterruptFlag);
    void wait(SDIOInterruptFlag);
    void check(SDIOInterruptFlag);
    // card register access functions
    void getOCR(void);
    void getCID(void);
    void getCSD(void);
    void getSCR(void);
    void setDSR(void);
    
    /** other functions to be developed
    void reset(void);
    void identify(void);
    void protect(void); // write protect
    void passwordSet(void);
    void passwordReset(void);
    void cardLock(void);
    void cardUnlock(void);
    void erase(void);
    */
};

#endif