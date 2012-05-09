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
    //delete SDIO;
}

/**
 * @brief Configure common startup settings 
 */
void SecureDigitalMemoryCard::begin(void) {
    sdio_set_clkcr(this->sdio_d, SDIO_CLK_INIT); 
    sdio_cfg_gpio(SDIO_BUS_INIT);
    sdio_init(this->sdio_d);
    delay(1);
}

/**
 * @brief Reset sdio device
 */
void SecureDigitalMemoryCard::end(void) {
    sdio_reset(this->sdio_d);
}

/**
 * @brief Card detect and debug function
 */
void SecureDigitalMemoryCard::cardDetect(void) {
    if (sdio_card_detect()) {
        SerialUSB.println("SDIO_DBG: Card detected");
    } else {
        SerialUSB.println("SDIO_DBG: Card not detected");
    }
}

/**
 * @brief Card Initialization and Identification Flow (SD mode)
 */
void SecureDigitalMemoryCard::init(void) {
    this->cardDetect();
    sdio_power_on(this->sdio_d);
    SerialUSB.println("SDIO_DBG: Powered on");
    delay(1);
    this->cmd(GO_IDLE_STATE); //CMD0
    //sdio_clear_interrupt(this->sdio_d, ~SDIO_ICR_RESERVED);
    SerialUSB.println("SDIO_DBG: Card in idle state");
    icr R7;
    CSD.version = CSD_UNDEFINED;
    this->cmd(SEND_IF_COND, //CMD8
              SDIO_HOST_SUPPLY_VOLTAGE | SDIO_CHECK_PATTERN,
              SDIO_WRSP_SHRT,
              (uint32*)&R7);
    uint32 arg;
    if (sdio_get_status(this->sdio_d, SDIO_STA_CMDREND)) {
        if ((R7.CHECK_PATTERN != SDIO_CHECK_PATTERN) |
            (R7.VOLTAGE_ACCEPTED != SDIO_HOST_SUPPLY_VOLTAGE)) {
            SerialUSB.println("SDIO_ERR: Unusuable Card");
            return;
        }
        SerialUSB.println("SDIO_DBG: Interface condition check passed");
        arg = SDIO_HOST_CAPACITY_SUPPORT;
    } else {
    /** the host should set HCS to 0 if the card returns no response to CMD8 */
        arg = 0;
        //prob version 1.x memory card
        CSD.version = CSD_VERSION1;
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
        CSD.version = CSD_VERSION2;
    } else {
        //version 2.00 or later High/Ext Capacity
        CSD.version = CSD_VERSION2;
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
void SecureDigitalMemoryCard::clockFreq(SDIOClockFrequency freq) {
    sdio_set_clkcr(this->sdio_d, ((uint32)freq) |
                   SDIO_CLKCR_WIDBUS_DEFAULT | SDIO_CLKCR_CLKEN);
}

/**
 * @brief Change bus width in host and card
 * @param width WIDBUS value to set
 */
void SecureDigitalMemoryCard::busMode(SDIOBusMode width) {
    //note: card bus can only be changed when card is unlocked
    sdio_cfg_gpio(width);
}

/**
 * @brief Set data block size for data commands
 * @param size 
 */
void SecureDigitalMemoryCard::blockSize(SDIOBlockSize size) {
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
    sdio_clear_interrupt(this->sdio_d, ~SDIO_ICR_RESERVED);
    sdio_cfg_interrupt(this->sdio_d, //SDIO_MASK_CMDACTIE |
                       SDIO_MASK_CMDSENTIE | SDIO_MASK_CMDRENDIE |
                       SDIO_MASK_CTIMEOUTIE | SDIO_MASK_CCRCFAILIE, 2);
    sdio_load_arg(this->sdio_d, arg);
    sdio_clock_enable(this->sdio_d);
    sdio_send_cmd(this->sdio_d, (wrsp << SDIO_CMD_WAITRESP_BIT) | cmd |
                  SDIO_CMD_CPSMEN | SDIO_CMD_IEN);
    if (sdio_is_cmd_act(this->sdio_d)) {
        SerialUSB.println("SDIO_DBG: Command active");
    }
    switch (wrsp) {
    case SDIO_WRSP_NONE:
        //check cmdsent
        SerialUSB.println("SDIO_DBG: Waiting for CMDSENT...");
        this->wait(SDIO_FLAG_CMDSENT);
        SerialUSB.println("SDIO_DBG: Command sent");
        //sdio_clear_interrupt(this->sdio_d, SDIO_ICR_CMDSENTC);
        break;
    case SDIO_WRSP_SHRT:
    case SDIO_WRSP_LONG:
        //wait for resp
        SerialUSB.println("SDIO_DBG: Waiting for CMDREND...");
        this->wait(SDIO_FLAG_CMDREND);
        SerialUSB.println("SDIO_DBG: Command response recieved");
        //sdio_clear_interrupt(this->sdio_d, SDIO_ICR_CMDRENDC);
        break;
    default:
        SerialUSB.println("SDIO_ERR: Invalid wait response");
        return;
    }
/**
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
    } //end of if statement
*/
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
    /**
    a)  Program the SDIO data length register (SDIO data timer register should
    be already programmed before the card identification process)
    b)  Program the SDIO argument register with the address location of the
    card where data is to be transferred
    c)  Program the SDIO command register: CmdIndex with 24 (WRITE_BLOCK);
    WaitResp with ‘1’ (SDIO card host waits for a response); CPSMEN with ‘1’
    (SDIO card host enabled to send a command). Other fields are at their
    reset value.
    d)  Wait for SDIO_STA[6] = CMDREND interrupt, then program the SDIO data
    control register: DTEN with ‘1’ (SDIO card host enabled to send data);
    DTDIR with ‘0’ (from controller to card); DTMODE with ‘0’ (block data 
    transfer); DMAEN with ‘1’ (DMA enabled); DBLOCKSIZE with 0x9 (512 bytes).
    Other fields are don’t care.
    e)  Wait for SDIO_STA[10] = DBCKEND
    */
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

/**
void SecureDigitalMemoryCard::clear(SDIOInterruptFlag flag) {
    switch (flag) {
    case SDIO_FLAG_ALL:
        sdio_clear_interrupt(this->sdio_d, ~SDIO_ICR_RESERVED);
        break;
    default:
        sdio_clear_interrupt(this->sdio_d, (0x1 << flag));
    }
}

void SecureDigitalMemoryCard::enable(SDIOInterruptFlag flag) {
    switch (flag) {
    case SDIO_FLAG_ALL:
        sdio_cfg_interrupt(this->sdio_d, ~SDIO_MASK_RESERVED, 2);
        break;
    default:
        sdio_cfg_interrupt(this->sdio_d, (0x1 << flag), 1);
    }
}

void SecureDigitalMemoryCard::disable(SDIOInterruptFlag flag) {
    switch (flag) {
    case SDIO_FLAG_ALL:
        sdio_cfg_interrupt(this->sdio_d, 0x0, 2);
        break;
    default:
        sdio_cfg_interrupt(this->sdio_d, (0x1 << flag), 0);
    }
}
*/

void SecureDigitalMemoryCard::wait(SDIOInterruptFlag flag) {
    while (!sdio_get_status(this->sdio_d, (0x1 << flag))) {
        if (sdio_get_status(this->sdio_d, SDIO_STA_CTIMEOUT)) {
            //sdio_clear_interrupt(this->sdio_d, SDIO_ICR_CTIMEOUTC);
            SerialUSB.println("SDIO_ERR: Command timeout");
            return;
        } else {
            SerialUSB.println(this->sdio_d->regs->STA, HEX);
            delay(100);
        }
    }
    sdio_clear_interrupt(this->sdio_d, (0x1 << flag));
}