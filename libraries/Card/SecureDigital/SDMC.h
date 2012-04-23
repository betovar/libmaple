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
 * @file SecureDigitalMemoryCard.h
 * @author Brian E Tovar <betovar@leaflabs.com>
 * @brief Wirish SD memory card implementation using HardwareSDIO. 
 */

#include "libmaple_types.h"
#include "HardwareSDIO.h"
#include "Structures.h"
#include "Commands.h"

#ifndef _SDMC_H_
#define _SDMC_H_

 #define SDIO_HOST_CAPACITY_SUPPORT (0x1 << 30)
 #define SDIO_FAST_BOOT             (0x1 << 29) //Reserved or eSD cards
 #define SDIO_SDXC_POWER_CONTROL    (0x1 << 28)
 #define SDIO_SWITCH_1V8_REQUEST    (0x1 << 24)
 #define SDIO_CHECK_PATTERN         0xAA
 #define SDIO_HOST_SUPPLY_VOLTAGE   0x1

 /**
 * @brief Wirish SecureDigitalMemoryCard interface
 */
class SecureDigitalMemoryCard : public HardwareSDIO {
  public:
    ocr OCR;
    cid CID;
    rca RCA;
    dsr DSR; // Default is 0x0404
    csd CSD;
    scr SCR;

    SecureDigitalMemoryCard();
    // common functions
    void init(void);
    void freq(SDIOFrequency);
    void bus(SDIODataBusWidth);
    void stop(void);
    void cmd(SDIOCommand);
    void cmd(SDIOCommand, uint32);
    void cmd(SDIOCommand, uint32, SDIOWaitResp, uint32*);
    void acmd(SDIOAppCommand);
    void acmd(SDIOAppCommand, uint32);
    void acmd(SDIOAppCommand, uint32, SDIOWaitResp, uint32*);
    // read and write data functions
    void readData(uint8*, uint32);
    void writeData(const uint8*, uint32);
    void readBlock(void);
    void writeBlock(void);

  private:
    void getOCR(void);
    void getCID(void);
    void getCSD(void);
    void getSCR(void);
    void getSSR(void);
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

    /** functions for UHS cards
    void voltageSwitchSequence(void); // CMD11
    void operatingVoltageValidation(void);
    */
};

#endif