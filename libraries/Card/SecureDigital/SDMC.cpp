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
    this->cmd(GO_IDLE_STATE);
    icr R7;
    this->cmd(SEND_IF_COND, 0x01AA, SDIO_WRSP_SHRT, (uint32*)&R7);
    if (R7.VOLTAGE_ACCEPTED !=  0x1) {
        ASSERT(0);
    }
    this->acmd(SD_SEND_OP_COND, 0, SDIO_WRSP_SHRT, (uint32*)&this->OCR);
    this->cmd(ALL_SEND_CID, 0, SDIO_WRSP_SHRT, (uint32*)&this->CID);
    this->cmd(SEND_RELATIVE_ADDR, 0, SDIO_WRSP_SHRT, (uint32*)&this->RCA);
}

/**
 * @brief Configure bus in clock control register and send command to card
 * @param width WIDBUS value to set
 */
void SecureDigitalMemoryCard::bus(SDIODataBusWidth width) {
    sdio_cfg_gpio(this->sdio_d, (uint8)width);
    /* WIDBUS: width of data bus is set */
    if (width <= 1) {
        sdio_cfg_clkcr(this->sdio_d, SDIO_CLKCR_WIDBUS, 
                      (width << SDIO_CLKCR_WIDBUS_BIT) );
    } else {
        ASSERT(0); //TODO[0.2.0] add support for UHS cards
    }
    // send command to set bus width in card: ACMD6 or (SDIO)CMD52
    this->acmd(SET_BUS_WIDTH, 0, SDIO_WRSP_SHRT, NULL);//FIXME
}

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
    if (R1.APP_CMD != 0) {
        ASSERT(0);
    } else {
        this->send(indx, arg, resp);
    }
}

/**
 * @brief Stop 
 */
void SecureDigitalMemoryCard::stop(void) {
    this->cmd(STOP_TRANSMISSION);
}

/*
 * Card register functions
 */

/**
 * @brief Load the Card IDentification Number into memory
 */
void SecureDigitalMemoryCard::getCID(void) {
    //this->cmd(SEND_CID, this.RCA, SDIO_WRSP_LONG, (void*)&this->CID);
}

/**
 * @brief Load the Card Specific Data into memory 
 */
void SecureDigitalMemoryCard::getCSD(void) {
    //this->cmd(SEND_CSD, this.RCA, SDIO_WRSP_LONG, (void*)&this->CSD);
}

/**
 * @brief Load the 
 */
void SecureDigitalMemoryCard::getSCR(void) {
    //this->cmd(SD_STATUS, 0, SDIO_WRSP_SHRT, (void*)&this->SCR);
}

/**
 * @brief Load the 
 */
void SecureDigitalMemoryCard::getCSR(void) {
    //this->acmd(APP_CMD, this.RCA, );
}

/**
 * @brief Load the 
 */
void SecureDigitalMemoryCard::getSSR(void) {

}

/*
 * Card convenience functions
 */


 /*
 * I/O
 */

