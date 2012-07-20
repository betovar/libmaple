/******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2012 LeafLabs.
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
 * @file SPIMode.h
 * @brief High-level SPI interface for SD cards
 */

#include <libmaple/libmaple_types.h>
#include <wirish/HardwareSPI.h>
#include "SDCard.h"
 
#ifndef _SPIMODE_H_
#define _SPIMODE_H_

class SPIMode : public SecureDigitalCard, public HardwareSPI {
  public:
    SPIMode(void);
    void begin(void);
    void end(void);
    void read(uint8 *dst);
    void write(const uint8 *src);
};

/*
class Sd2Card : public HardwareSPI {
    public:
        Sd2Card();
        void spiSend(uint8 b);
        uint8 spiRec();
        uint32 cardSize(void);
        uint8 erase(uint32 firstBlock, uint32 lastBlock);
        uint8 eraseSingleBlockEnable(void);
        uint8 errorCode(void);
        uint8 errorData(void);
        uint8 init();
        void partialBlockRead(uint8 value);
        uint8 partialBlockRead(void);
        uint8 readBlock(uint32 block, uint8* dst);
        uint8 readData(uint32 block, uint16 offset, uint16 count, uint8* dst);
        uint8 readCID(cid_t* cid);
        uint8 readCSD(csd_t* csd);
        void readEnd(void);
        uint8 type(void);
        uint8 writeBlock(uint32 blockNumber, const uint8* src);
        uint8 writeData(const uint8* src);
        uint8 writeStart(uint32 blockNumber, uint32 eraseCount);
        uint8 writeStop(void);
 
    private:
        uint32_t block_;
        uint8_t chipSelectPin_;
        uint8_t errorCode_;
        uint8_t inBlock_;
        uint16_t offset_;
        uint8_t partialBlockRead_;
        uint8_t status_;
        uint8_t type_;
        //pol
        uint8_t ack[SPI_BUFF_SIZE];
        // private functions
        uint8_t cardAcmd(uint8_t cmd, uint32_t arg) {
            cardCommand(CMD55, 0);
            return cardCommand(cmd, arg);
        }
        uint8_t cardCommand(uint8_t cmd, uint32_t arg);
        void error(uint8_t code) {errorCode_ = code;}
        uint8_t readRegister(uint8_t cmd, void* buf);
        uint8_t sendWriteCommand(uint32_t blockNumber, uint32_t eraseCount);
        void type(uint8_t value) {type_ = value;}
        uint8_t waitNotBusy(uint16_t timeoutMillis);
        uint8_t writeData(uint8_t token, const uint8_t* src);
        uint8_t waitStartBlock(void);
};
*/

#endif