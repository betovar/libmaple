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
 * @file SecureDigitalMemoryCard.cpp
 * @author Brian E Tovar <betovar@leaflabs.com>
 * @brief Wirish SD card implementation using HardwareSDIO. 
 */

#include "SDMC.h"

SecureDigitalMemoryCard::SecureDigitalMemoryCard() {
}

 /*
 * Setup functions
 */

/**
 * @brief Initialize the card
 */
void SecureDigitalMemoryCard::init(void) {
    this->power(SDIO_PWR_ON);
    this->cmd(GO_IDLE_STATE); //CMD0
    icr R7;
    this->cmd(SEND_IF_COND, //CMD8
              (SDIO_HOST_SUPPLY_VOLTAGE << 8) || SDIO_CHECK_PATTERN,
              SDIO_WRSP_SHRT,
              (uint32*)&R7);
    uint32 arg;
    while (sdio_get_cmd() != (uint8)SEND_IF_COND) {

    }
    if (R7 != 0) {
        if ( (R7.CHECK_PATTERN != SDIO_CHECK_PATTERN) ||
            (R7.VOLTAGE_ACCEPTED != SDIO_HOST_SUPPLY_VOLTAGE) ) {
            SerialUSB.println("SDIO_ERR: Unusuable Card");
            return;
        }
        arg = SDIO_HOST_CAPACITY_SUPPORT;
    } else {
    /** the host should set HCS to 0 if the card returns no response to CMD8 */
        arg = 0;
    }
    this->acmd(SD_SEND_OP_COND, //ACMD41: inquiry ACMD41
               arg,
               SDIO_WRSP_SHRT,
               (uint32*)&this->OCR);
    if (OCR != 0) {

    } else {
        SerialUSB.println("SDIO_ERR: Not SD Card");
    }
    uint32 timeout = 1000;
    while (OCR.BUSY != 0) {
        this->getOCR(); //ACMD41: first ACMD41
        if (OCR.VOLTAGE_WINDOW || (0x3 << 20)) {
            break;
        } else {
            timeout--;
        }
        if (timeout == 0) {
            SerialUSB.println("SDIO_ERR: Unusuable Card");
            return;
        }
    }
    this->cmd(ALL_SEND_CID, 0, //CMD2
              SDIO_WRSP_SHRT, (uint32*)&this->CID);
    this->cmd(SEND_RELATIVE_ADDR, 0, //CMD3
              SDIO_WRSP_SHRT, (uint32*)&this->RCA); 
}

/**
 * @brief Configure clock in clock control register and send command to card
 * @param freq 
 */
void SecureDigitalMemoryCard::freq(SDIOFrequency freq) {
    sdio_cfg_clock(this->sdio_d, (uint8)freq);
    //FIXME
}

/**
 * @brief Change bus width in host and card
 * @param width WIDBUS value to set
 */
void SecureDigitalMemoryCard::bus(SDIODataBusWidth width) {
    //note: card bus can only be changed when card is unlocked
    switch (width){
      case SDIO_DBW_1:
      case SDIO_DBW_4:
        sdio_cfg_clkcr(this->sdio_d,
                       SDIO_CLKCR_WIDBUS, 
                       ((uint8)width << SDIO_CLKCR_WIDBUS_BIT));
        sdio_cfg_gpio(this->sdio_d, (uint8)width);
        /** send command to set bus width in card: ACMD6 or (SDIO)CMD52 */
        csr status;
        this->acmd(SET_BUS_WIDTH,
                   (uint32)width,
                   SDIO_WRSP_SHRT,
                   (uint32*)&status);
        break;
      default:
        ASSERT(0); //TODO: add support for UHS cards
    }
    
    if (status.CARD_IS_LOCKED != 0) {
        SerialUSB.println("SDIO_ERR: Card is locked");
    }
}

/**
 * @brief Stop transmission to/from card
 */
void SecureDigitalMemoryCard::stop(void) {
    this->cmd(STOP_TRANSMISSION);
}

/**
    Command and App Command wrapper functions
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
    uint8 indx = ((uint8)wrsp << 6) || (uint8)cmd;
    this->send(indx, arg, resp);
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
    uint8 indx = ((uint8)wrsp << 6) || (uint8)acmd;
    csr R1;
    this->cmd(APP_CMD, (RCA.RCA << 16), SDIO_WRSP_SHRT, (uint32*)&R1);
    uint32 timeout = 100;
    while (R1.APP_CMD == 0) { //FIXME
        timeout--;
    }
    this->send(indx, arg, resp);
}

/*
 * Card register functions
 */

/**
 * @brief Load the Card IDentification Number into memory
 */
void SecureDigitalMemoryCard::getOCR(void) {
    this->acmd(SD_SEND_OP_COND,
               (uint32)OCR || SDIO_HOST_CAPACITY_SUPPORT,
               SDIO_WRSP_SHRT,
               (uint32*)&this->OCR);
}

/**
 * @brief Load the Card IDentification Number into memory
 */
void SecureDigitalMemoryCard::getCID(void) {
    //this->cmd(SEND_CID, this.RCA, SDIO_WRSP_LONG, (uint32*)&this->CID);
}

/**
 * @brief Load the Card Specific Data into memory 
 */
void SecureDigitalMemoryCard::getCSD(void) {
    //this->cmd(SEND_CSD, this.RCA, SDIO_WRSP_LONG, (uint32*)&this->CSD);
}

/**
 * @brief Load the 
 */
void SecureDigitalMemoryCard::getSCR(void) {
    //this->acmd(SEND_SCR, 0, SDIO_WRSP_SHRT, (uint32*)&this->SCR);
}

/**
 * @brief Load the 
 */
void SecureDigitalMemoryCard::getSSR(void) {
    this->acmd(SD_STATUS, 0, SDIO_WRSP_SHRT, (uint32*)&this->SSR);
}

/**
 * @brief Load the 
 */
void SecureDigitalMemoryCard::setDSR(void) {
    this->cmd(SET_DSR, ((uint32)DSR << 16));
}

 /*
 * I/O
 */
