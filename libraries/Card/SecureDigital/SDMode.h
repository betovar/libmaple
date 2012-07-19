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
 * @file SDMode.h
 * @author Brian E Tovar <betovar@leaflabs.com>
 * @brief Wirish SD Memory Card implementation
 */

#include <libmaple/sdio.h>
#include <libmaple/libmaple_types.h>
#include <libraries/Card/SD/SDCard.h>


#ifndef _SDMODE_H_
#define _SDMODE_H_

/**
 * SDIO Enumerations
 */

typedef enum SDIORespType {
    SDIO_RESP_NONE     = 0,
    SDIO_RESP_SHORT    = 1,
  //SDIO_RESP_NONE_2   = 2,
    SDIO_RESP_LONG     = 3,
    SDIO_RESP_TYPE1,
  //SDIO_RESP_TYPE1b,
    SDIO_RESP_TYPE2,
    SDIO_RESP_TYPE3,
  //SDIO_RESP_TYPE4,
  //SDIO_RESP_TYPE5,
    SDIO_RESP_TYPE6,
    SDIO_RESP_TYPE7
} SDIORespType;

typedef enum SDIOBusMode {
    SDIO_BUS_1BIT     = 0,
    SDIO_BUS_4BIT     = 1
  //SDIO_BUS_8BIT     = 2
} SDIOBusMode;

typedef enum SDIOClockFrequency {
  //SDIO_36_MHZ   = 0, // High Speed Mode not supported as of May 2012
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
    SDIO_BKSZ_1       = 0,
    SDIO_BKSZ_2       = 1,
    SDIO_BKSZ_4       = 2,
    SDIO_BKSZ_8       = 3,
    SDIO_BKSZ_16      = 4,
    SDIO_BKSZ_32      = 5,
    SDIO_BKSZ_64      = 6,
    SDIO_BKSZ_128     = 7,
    SDIO_BKSZ_256     = 8,
    SDIO_BKSZ_512     = 9,
    SDIO_BKSZ_1024    = 10,
    SDIO_BKSZ_2048    = 11,
    SDIO_BKSZ_4096    = 12,
    SDIO_BKSZ_8192    = 13,
    SDIO_BKSZ_16384   = 14,
    SDIO_BKSZ_DEFAULT = 9
} SDIOBlockSize;

typedef enum SDIOInterruptFlag {
    SDIO_FLAG_CCRCFAIL  = 0,
    SDIO_FLAG_DCRCFAIL  = 1,
    SDIO_FLAG_CTIMEOUT  = 2,
    SDIO_FLAG_DTIMEOUT  = 3,
    SDIO_FLAG_TXUNDERR  = 4,
    SDIO_FLAG_RXOVERR   = 5,
    SDIO_FLAG_CMDREND   = 6,
    SDIO_FLAG_CMDSENT   = 7,
    SDIO_FLAG_DATAEND   = 8,
    SDIO_FLAG_STBITERR  = 9,
    SDIO_FLAG_DBCKEND   = 10,
    SDIO_FLAG_CMDACT    = 11,
    SDIO_FLAG_TXACT     = 12,
    SDIO_FLAG_RXACT     = 13,
    SDIO_FLAG_TXFIFOHE  = 14,
    SDIO_FLAG_RXFIFOHF  = 15,
    SDIO_FLAG_TXFIFOF   = 16,
    SDIO_FLAG_RXFIFOF   = 17,
    SDIO_FLAG_TXFIFOE   = 18,
    SDIO_FLAG_RXFIFOE   = 19,
    SDIO_FLAG_TXDAVL    = 20,
    SDIO_FLAG_RXDAVL    = 21,
    SDIO_FLAG_SDIOIT    = 22,
    SDIO_FLAG_CEATAEND  = 23,
    // added
    SDIO_FLAG_NONE      = 32,
    SDIO_FLAG_ERROR
} SDIOInterruptFlag;

/**
 * Hardware SDIO class definition
 */

class SDMode : public SecureDigitalCard { // HardwareSDIO
  public:
    sdio_dev *sdio_d;

    SDMode(void);
    void begin(void);
    void end(void);
    void read(uint8 *dst);
    void write(const uint8 *src);
  protected:  
    //---------------- convenience functions --------------
    void clockFreq(SDIOClockFrequency);
    void busMode(SDIOBusMode);
    void blockSize(SDIOBlockSize);
    void select(uint16);
    void select(void);
    void deselect(void);
    //---------------- command functions ------------------
    void command(SDIOCommand);
    void command(SDIOCommand, uint32);
    void command(SDIOCommand, uint32, SDIORespType, uint32*);
    void command(SDIOAppCommand);
    void command(SDIOAppCommand, uint32);
    void command(SDIOAppCommand, uint32, SDIORespType, uint32*);
    //---------------- public card register access functions
    void getOCR(void); //only allowed during identification mode
    void getCID(void);
    void getCSD(void);
    void getSCR(void);
    void getSSR(uint32*);
    void setDSR(void);
    void newRCA(void);
    //---------------- basic data functions ---------------
    void readBlock(uint32, uint32*);
    void writeBlock(uint32, const uint32*);
    void stop(void);
    //---------------- start routines ---------------------
    void idle(void);
    void initialization(void);
    void identification(void);
    
    /** other functions to be developed
    void protect(void); // write protect
    void password(void); // password set/reset
    void lock(void);
    void unlock(void);
    void erase(void);
    */

  /** 
  Sd2Card(void) : errorCode_(0), inBlock_(0), partialBlockRead_(0), type_(0) {}
  uint32_t cardSize(void);
  uint8_t erase(uint32_t firstBlock, uint32_t lastBlock);
  uint8_t eraseSingleBlockEnable(void);
  uint8_t errorCode(void);
  uint8_t errorData(void);
  uint8_t init(void);
  uint8_t init(uint8_t sckRateID);
  uint8_t init(uint8_t sckRateID, uint8_t chipSelectPin);
  void partialBlockRead(uint8_t value);
  uint8_t partialBlockRead(void) const {return partialBlockRead_;}
  uint8_t readBlock(uint32_t block, uint8_t* dst);
  uint8_t readData(uint32_t block,
          uint16_t offset, uint16_t count, uint8_t* dst);
  uint8_t readCID(cid_t* cid);
  uint8_t readCSD(csd_t* csd);
  void readEnd(void);
  uint8_t setSckRate(uint8_t sckRateID);
  uint8_t type(void)
  uint8_t writeBlock(uint32_t blockNumber, const uint8_t* src);
  uint8_t writeData(const uint8_t* src);
  uint8_t writeStart(uint32_t blockNumber, uint32_t eraseCount);
  uint8_t writeStop(void);
 private:
  uint32_t block_;
  uint8_t chipSelectPin_;
  uint8_t errorCode_;
  uint8_t inBlock_;
  uint16_t offset_;
  uint8_t partialBlockRead_;
  uint8_t status_;
  uint8_t type_;
  // private functions
  uint8_t cardAcmd(uint8_t cmd, uint32_t arg);
  uint8_t cardCommand(uint8_t cmd, uint32_t arg);
  void error(uint8_t code) {errorCode_ = code;}
  uint8_t readRegister(uint8_t cmd, void* buf);
  uint8_t sendWriteCommand(uint32_t blockNumber, uint32_t eraseCount);
  void chipSelectHigh(void);
  void chipSelectLow(void);
  void type(uint8_t value) {type_ = value;}
  uint8_t waitNotBusy(uint16_t timeoutMillis);
  uint8_t writeData(uint8_t token, const uint8_t* src);
  uint8_t waitStartBlock(void);
  */
};

#endif