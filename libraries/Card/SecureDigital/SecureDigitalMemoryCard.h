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
#include "Structures.h"
#include "Commands.h"
#include "HardwareSDIO.h"

#ifndef _SECUREDIGITALMEMORYCARD_H_
#define _SECUREDIGITALMEMORYCARD_H_
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
    csr CSR;
    ssr SSR;

    SecureDigitalMemoryCard();

/*
 * I/O
 */

    /**
     * @brief 
     * @param 
     */
    void readData(uint8 *buffer, uint32 length);

    /**
     * @brief 
     * @param 
     */
    void writeData(const uint8 *buffer, uint32 length);

    void readBlock(void);
    void writeBlock(void);

  private:
    void busWidth(uint8 width);

    void getCID(void);
    void getCSD(void);
    void getCSR(void);
    void getSSR(void);

    //void broadcast_cmd(uint8 cmd);
    //void broadcast_cmd_wresponse(uint8 cmd);
    //void addr_cmd(uint8 cmd);
    //void addr_data_xfer_cmd(uint8 cmd);
    
    void wide_bus_selection(void); // ACMD6
    void host_reset(void);
    void card_initialization(void); // ACMD41
    void card_identification_process(void);
    //void operating_voltage_validation(void);

    void protect(void); // write protect
    void passwordSet(void);
    void passwordReset(void);
    void cardLock(void);
    void cardUnlock(void);
    void stop(void); // CMD12
    void erase(void);


    /** functions for UHS cards
    void voltageSwitchSequence(void); // CMD11
    */
};

#endif