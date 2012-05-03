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
 * @file SDMC.cpp
 * @author Brian E Tovar <betovar@leaflabs.com>
 * @brief Wirish SD Memory Card implementation
 */

#include "SDMC.h"
#include "wirish.h"
#include "util.h"
#include "rcc.h"

#define SDIO_HOST_CAPACITY_SUPPORT (0x1 << 30)
#define SDIO_FAST_BOOT             (0x1 << 29) //Reserved or eSD cards
#define SDIO_SDXC_POWER_CONTROL    (0x1 << 28) 
#define SDIO_SWITCH_1V8_REQUEST    (0x1 << 24) //Not allowed on STM32
#define SDIO_CHECK_PATTERN          0xAA       //Recommended but not fixed
#define SDIO_HOST_SUPPLY_VOLTAGE   (0x1 << 8)

#if CYCLES_PER_MICROSECOND != 72
/* TODO [0.2.0?] something smarter than this */
#warning "Unexpected clock speed; SDIO frequency calculation will be incorrect"
#endif

/**
 * @brief Constructor
 */
SecureDigitalMemoryCard::SecureDigitalMemoryCard() {
    this->sdio_d = SDIO;
}

/**
 * @brief Configure common startup routines
 */
void SecureDigitalMemoryCard::begin(void) {
    sdio_cfg_clkcr(this->sdio_d, SDIO_CLKCR_WIDBUS,
                   (SDIO_DBW_1 << SDIO_CLKCR_WIDBUS_BIT));
    sdio_cfg_gpio(this->sdio_d, SDIO_DBW_1);
    sdio_set_clkcr(this->sdio_d, SDIO_INIT_FREQ |
                   SDIO_CLKCR_WIDBUS_DEFAULT | SDIO_CLKCR_CLKEN);
    sdio_init(this->sdio_d);
}

/**
 * @brief 
 */
void SecureDigitalMemoryCard::end(void) {
    sdio_reset(this->sdio_d);
}

/**
 * @brief Set the condition of the power register
 */
void SecureDigitalMemoryCard::power(SDIOPowerState pwr) {
    /** At least seven HCLK clock periods are needed between two write accesses
     to this register */
    switch (pwr) {
    case SDIO_PWR_ON:
    case SDIO_PWR_OFF:
        sdio_power(this->sdio_d, (uint32)pwr);
        break;
    default:
        SerialUSB.println("SDIO_ERR: Invalid power state");
    }//end of switch
}


/**
 * @brief Card Initialization and Identification Flow (SD mode)
 */
void SecureDigitalMemoryCard::init(void) {
    this->power(SDIO_PWR_ON);
    SerialUSB.println("SDIO_DBG: card powered on");
    this->cmd(GO_IDLE_STATE); //CMD0
    SerialUSB.println("SDIO_DBG: card in idle state");
    icr R7;
    CSD.version = 0;
    this->cmd(SEND_IF_COND, //CMD8
              SDIO_HOST_SUPPLY_VOLTAGE | SDIO_CHECK_PATTERN,
              SDIO_WRSP_SHRT,
              (uint32*)&R7);
    SerialUSB.println("SDIO_DBG: got interface condition");
    uint32 arg;
    if (sdio_get_status(this->sdio_d, SDIO_STA_CMDREND)) {
        if ((R7.CHECK_PATTERN != SDIO_CHECK_PATTERN) |
            (R7.VOLTAGE_ACCEPTED != SDIO_HOST_SUPPLY_VOLTAGE)) {
            SerialUSB.println("SDIO_ERR: Unusuable Card");
            return;
        }
        arg = SDIO_HOST_CAPACITY_SUPPORT;
    } else {
    /** the host should set HCS to 0 if the card returns no response to CMD8 */
        arg = 0;
        //prob version 1.x memory card
        CSD.version = 1;
    }
    this->acmd(SD_SEND_OP_COND, //ACMD41: inquiry ACMD41
               arg,
               SDIO_WRSP_SHRT,
               (uint32*)&this->OCR);
    if (!sdio_get_status(this->sdio_d, SDIO_STA_CMDREND)) {
        SerialUSB.println("SDIO_ERR: Not SD Card");
        return;
    }
    while (OCR.BUSY != 1) {
        this->getOCR(); //ACMD41: first ACMD41
    }
    if ((OCR.VOLTAGE_WINDOW && 0x300000) != 0) {
        SerialUSB.println("SDIO_ERR: Unusuable Card");
        return;
    }
    if (OCR.CCS == 0) {
        //version 2.00 or later Std Capacity
        CSD.version = 2;
    } else {
        //version 2.00 or later High/Ext Capacity
        CSD.version = 2;
    }
    /** Voltage Switch not supported
    if (OCR.S18A == 1) {
        this->cmd(VOLTAGE_SWITCH, 0, //CMD11
                  SDIO_WRSP_SHRT, (uint32*)&this->CID);
    }
    */
    this->cmd(ALL_SEND_CID, 0, //CMD2
              SDIO_WRSP_SHRT, (uint32*)&this->CID);
    this->cmd(SEND_RELATIVE_ADDR, 0, //CMD3
              SDIO_WRSP_SHRT, (uint32*)&this->RCA);
    this->getCSD();
    //this->getSCR();
}

/**
 * @brief Configure clock in clock control register and send command to card
 * @param freq 
 */
void SecureDigitalMemoryCard::clockFreq(SDIOFrequency freq) {
    sdio_set_clkcr(this->sdio_d, ((uint32)freq) |
                   SDIO_CLKCR_WIDBUS_DEFAULT | SDIO_CLKCR_CLKEN);
}

/**
 * @brief Change bus width in host and card
 * @param width WIDBUS value to set
 */
void SecureDigitalMemoryCard::busWidth(SDIODataBusWidth width) {
    //note: card bus can only be changed when card is unlocked
    switch (width){
    case SDIO_DBW_1:
    case SDIO_DBW_4:
        sdio_cfg_clkcr(this->sdio_d, SDIO_CLKCR_WIDBUS,
                       ((uint32)width << SDIO_CLKCR_WIDBUS_BIT));
        sdio_cfg_gpio(this->sdio_d, (uint8)width);
        /** send command to set bus width in card: ACMD6 or (SDIO)CMD52 */
        csr status;
        this->acmd(SET_BUS_WIDTH, (uint32)width,
                   SDIO_WRSP_SHRT, (uint32*)&status);
        if (status.CARD_IS_LOCKED == SDIO_CSR_CARD_LOCKED) {
            SerialUSB.println("SDIO_ERR: Card is locked");
            return;
        }
        break;
    default:
        //TODO: add support for UHS cards
        SerialUSB.println("SDIO_ERR: Unsupported bus width");
    }
}

/**
 * @brief Set data block size for data commands
 * @param size 
 */
void SecureDigitalMemoryCard::blockSize(SDIODataBlockSize size) {
    csr status;
    this->cmd(SET_BLOCKLEN, (0x1 << size),
              SDIO_WRSP_SHRT, (uint32*)&status);
    if (status.ERROR == SDIO_CSR_ERROR) {
        SerialUSB.println("SDIO_ERR: error in SET_BLOCKLEN respsonse");

    }
}

/**
 * Command and App Command wrapper functions
*/

/**
 * @brief Command (without response nor argument) to send to card
 * @param cmd Command index to send
 */
void SecureDigitalMemoryCard::cmd(SDIOCommand cmd) {
    this->cmd(cmd, 0, SDIO_WRSP_NONE, NULL);
}

/**
 * @brief Command (without response) to send to card
 * @param cmd Command index to send
 * @param arg Argument to send
 */
void SecureDigitalMemoryCard::cmd(SDIOCommand cmd, uint32 arg) {
    this->cmd(cmd, arg, SDIO_WRSP_NONE, NULL);
}

/**
 * @brief Command (with response) to send to card
 * @param cmd Command index to send
 * @param arg Argument to send
 * @param wrsp Wait for response tag 
 * @param resp Buffer to store response
 */
void SecureDigitalMemoryCard::cmd(SDIOCommand cmd,
                                  uint32 arg,
                                  SDIOWaitResp wrsp,
                                  uint32 *resp) {
    sdio_cfg_interrupt(this->sdio_d, SDIO_MASK_CMDACTIE |
                       SDIO_MASK_CMDSENTIE | SDIO_MASK_CMDRENDIE |
                       SDIO_MASK_CTIMEOUTIE | SDIO_MASK_CCRCFAILIE, 1);
    sdio_clear_interrupt(this->sdio_d, ~SDIO_ICR_RESERVED);
    sdio_load_arg(this->sdio_d, arg);
    sdio_send_cmd(this->sdio_d, (wrsp << 6) | cmd |
                  SDIO_CMD_CPSMEN | SDIO_CMD_IEN);
    while (sdio_xfer_in_prog(this->sdio_d)) {};
    switch (wrsp) {
    case SDIO_WRSP_NONE:
        //check cmdsent
        while (sdio_get_status(this->sdio_d, SDIO_STA_CMDSENT) != 1) {
            if (sdio_get_status(this->sdio_d, SDIO_STA_CTIMEOUT) == 1) {
                //sdio_clear_interrupt(this->sdio_d, SDIO_ICR_CTIMEOUTC);
                SerialUSB.println("SDIO_ERR: Command timeout");
                return;
            }
        }
        sdio_clear_interrupt(this->sdio_d, SDIO_ICR_CMDSENTC);
        break;
    case SDIO_WRSP_SHRT:
    case SDIO_WRSP_LONG:
        //wait for resp
        while (sdio_get_status(this->sdio_d, SDIO_STA_CMDREND) != 1) {
            if (sdio_get_status(this->sdio_d, SDIO_STA_CTIMEOUT) == 1) {
                //sdio_clear_interrupt(this->sdio_d, SDIO_ICR_CTIMEOUTC);
                SerialUSB.println("SDIO_ERR: Command timeout");
                return;
            }
        }
        //sdio_clear_interrupt(this->sdio_d, SDIO_ICR_CMDRENDC);
        break;
    default:
        SerialUSB.println("SDIO_ERR: Invalid wait response");
        return;
    }
    //check crc
    if (sdio_get_status(this->sdio_d, SDIO_STA_CCRCFAIL) != 1) {
        switch (wrsp) {
        case SDIO_WRSP_SHRT:
            sdio_get_resp_short(this->sdio_d, (uint32*)resp);
            break;
        case SDIO_WRSP_LONG:
            sdio_get_resp_long(this->sdio_d, (uint32*)resp);
            break;
        case SDIO_WRSP_NONE:
        default:
            break;
        }
        //sdio_clear_interrupt(this->sdio_d, SDIO_ICR_CCRCFAILC);
    }
}

/**
 * @brief Stop transmission to/from card
 */
void SecureDigitalMemoryCard::stop(void) {
    csr status;
    this->cmd(STOP_TRANSMISSION, 0, SDIO_WRSP_SHRT, (uint32*)&status);
    if (status.ERROR != SDIO_CSR_ERROR) {
        //FIXME
    }
}

/**
 * @brief Application Command (without response nor argument) to send to card
 * @param acmd Application Command to send
 */
void SecureDigitalMemoryCard::acmd(SDIOAppCommand acmd) {
    this->acmd(acmd, 0, SDIO_WRSP_NONE, NULL);
}

/**
 * @brief Application Command (without response) to send to card
 * @param acmd Command to send
 * @param arg Argument to send
 */
void SecureDigitalMemoryCard::acmd(SDIOAppCommand acmd,
                                   uint32 arg) {
    this->acmd(acmd, arg, SDIO_WRSP_NONE, NULL);
}

/**
 * @brief Application Command (with response) to send to card
 * @param acmd Application Command to send
 * @param arg Argument to send
 * @param wrsp Wait for response tag 
 * @param resp Buffer to store response
 */
void SecureDigitalMemoryCard::acmd(SDIOAppCommand acmd,
                                   uint32 arg,
                                   SDIOWaitResp wrsp,
                                   uint32 *resp) {
    csr status;
    this->cmd(APP_CMD, (RCA.RCA << 16), SDIO_WRSP_SHRT, (uint32*)&status);
    if (status.APP_CMD == SDIO_CSR_APP_ENABLED) { //FIXME
        this->cmd((SDIOCommand)acmd, arg, wrsp, resp);
    }
    
}

/**
 * Card register functions
 */

/**
 * @brief Load the Card IDentification Number into memory
 */
void SecureDigitalMemoryCard::getOCR(void) {
    this->acmd(SD_SEND_OP_COND,
               OCR.VOLTAGE_WINDOW | SDIO_HOST_CAPACITY_SUPPORT,
               SDIO_WRSP_SHRT,
               (uint32*)&this->OCR);
}

/**
 * @brief Load the Card IDentification Number into memory
 */
void SecureDigitalMemoryCard::getCID(void) {
    this->cmd(SEND_CID, ((uint32)RCA.RCA << 16),
              SDIO_WRSP_LONG, (uint32*)&this->CID);
}

/**
 * @brief Load the Card Specific Data into memory 
 */
void SecureDigitalMemoryCard::getCSD(void) {
    this->cmd(SEND_CSD, ((uint32)RCA.RCA << 16),
              SDIO_WRSP_LONG, (uint32*)&this->CSD);
}

/**
 * @brief Load the 
 */
void SecureDigitalMemoryCard::getSCR(void) {
    this->acmd(SEND_SCR, 0, SDIO_WRSP_SHRT, (uint32*)&this->SCR);
}

/**
 * @brief Load the 
 */
void SecureDigitalMemoryCard::setDSR(void) {
    this->cmd(SET_DSR, ((uint32)DSR << 16));
}

/**
 * Data Functions
 */

 /**
 * @brief Read next word from FIFO 
 * @retval Data that was read from FIFO
 */
void SecureDigitalMemoryCard::read(uint32 addr,
                                   uint32 *buf,
                                   uint32 count) {
    uint32 rxed = 0;
    while (rxed < count) {
        buf[rxed++] = sdio_read_data(this->sdio_d);
    }//FIXME add cmd with blcok addr
}

/**
 * @brief Write next word into FIFO
 * @param word Data to write to FIFO
 */
void SecureDigitalMemoryCard::write(uint32 addr, 
                                    const uint32 *buf,
                                    uint32 count) {
    uint32 txed = 0;
    while (txed < count) {
        sdio_write_data(this->sdio_d, buf[txed++]);
    }//FIXME add cmd with blcok addr
}

/**
 * @brief 
 * @param buf Buffer to save data to
 * @param addr Block address to read from
 */
void SecureDigitalMemoryCard::readBlock(uint32 addr, uint32 *buf) {
    sdio_cfg_interrupt(this->sdio_d, SDIO_MASK_RXFIFOFIE |
                       SDIO_MASK_RXFIFOEIE | SDIO_MASK_RXFIFOHFIE |
                       SDIO_MASK_RXDAVLIE | SDIO_MASK_RXOVERRIE, 1);
    //uint32 count = 512/4;
    csr status;
    this->cmd(READ_SINGLE_BLOCK, addr,
              SDIO_WRSP_SHRT, (uint32*)&status);
    if (sdio_get_status(this->sdio_d, SDIO_ICR_CMDRENDC) == 1) {
        //while (sdio_get_fifo_count(this->sdio_d)) 
        for (int i = 0; i < 8; i++) {
            buf[i] = sdio_read_data(this->sdio_d);
        }
    }
    
}
void SecureDigitalMemoryCard::writeBlock(uint32 addr, const uint32 *buf) {
    sdio_cfg_interrupt(this->sdio_d, SDIO_MASK_TXFIFOFIE |
                       SDIO_MASK_TXFIFOEIE | SDIO_MASK_TXFIFOHEIE |
                       SDIO_MASK_TXDAVLIE | SDIO_MASK_TXUNDERRIE, 1);
    //uint32 count = 512/4;
    csr status;
    this->cmd(WRITE_BLOCK, addr,
              SDIO_WRSP_SHRT, (uint32*)&status);
    if (sdio_get_status(this->sdio_d, SDIO_ICR_CMDRENDC) == 1) {
        //while (sdio_get_fifo_count(this->sdio_d)) 
        for (int i = 0; i < 8; i++) {
            sdio_write_data(this->sdio_d, buf[i]);
        }
    }
}

