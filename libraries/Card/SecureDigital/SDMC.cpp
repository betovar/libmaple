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

static const uint32 SDIO_HOST_CAPACITY_SUPPORT = 0x1 << 30;
static const uint32 SDIO_FAST_BOOT             = 0x1 << 29; //Reserved
static const uint32 SDIO_SDXC_POWER_CONTROL    = 0x1 << 28; 
static const uint32 SDIO_SWITCH_1V8_REQUEST    = 0x1 << 24; //Not allowed
static const uint32 SDIO_CHECK_PATTERN         = 0xAA;        //Not fixed
static const uint32 SDIO_VOLTAGE_SUPPLIED      = 0x1;
static const uint32 SDIO_VOLTAGE_HOST_SUPPORT  = SDIO_VOLTAGE_SUPPLIED << 8;
static const uint32 SDIO_VALID_VOLTAGE_WINDOW  = 0xFF80;

#if CYCLES_PER_MICROSECOND != 72
/* TODO [0.2.0?] something smarter than this */
#warning "Unexpected clock speed; SDIO frequency calculation will be incorrect"
#endif

/**
 * @brief Constructor
 */
SecureDigitalMemoryCard::SecureDigitalMemoryCard() {
    this->sdio_d = SDIO;
    this->RCA.RCA = 0x0;
    this->CSD.version = CSD_VER_UNDEF;
}

/**
 * @brief Configure common startup settings 
 */
void SecureDigitalMemoryCard::begin(void) {
    sdio_set_clkcr(this->sdio_d, SDIO_CLK_INIT); 
    sdio_cfg_gpio(SDIO_BUS_INIT);
    sdio_init(this->sdio_d);
}

/**
 * @brief Reset sdio device
 */
void SecureDigitalMemoryCard::end(void) {
    sdio_reset(this->sdio_d);
    delay(1);
    sdio_power_off(this->sdio_d);
    sdio_clock_disable(this->sdio_d);
}

/**
 * @brief Card Initialization and Identification Flow (SD mode)
 */
void SecureDigitalMemoryCard::init(void) {
    SerialUSB.println("SDIO_DBG: Initializing card");
    if (sdio_card_detect()) {
        SerialUSB.println("SDIO_DBG: Card detected");
    } else {
        SerialUSB.println("SDIO_ERR: Card not detected");
        return;
    }
    sdio_power_on(this->sdio_d);
    SerialUSB.println("SDIO_DBG: Powered on");
    sdio_clock_enable(this->sdio_d);
    delay(1);//Microseconds(185);
// -------------------------------------------------------------------------
    this->cmd(GO_IDLE_STATE); //CMD0
    SerialUSB.println("SDIO_DBG: Card in idle state");
// -------------------------------------------------------------------------
    icr status8;
    this->cmd(SEND_IF_COND, //CMD8
              //SDIO_SDXC_POWER_CONTROL |
              SDIO_VOLTAGE_HOST_SUPPORT | SDIO_CHECK_PATTERN,
              SDIO_RESP_SHRT,
              (uint32*)&status8);
    uint32 arg = SDIO_HOST_CAPACITY_SUPPORT;
    if (sdio_get_status(this->sdio_d, SDIO_STA_CTIMEOUT)) {
        SerialUSB.println("SDIO_ERR: No response from CMD8");
        // the host should set HCS to 0 if the card returns no response
        arg &= ~SDIO_HOST_CAPACITY_SUPPORT;
        //prob version 1.x memory card
        CSD.version = CSD_VER_1;
    } else if (sdio_get_status(this->sdio_d, SDIO_STA_CMDREND)) {
        if (status8.CHECK_PATTERN != SDIO_CHECK_PATTERN) {
            SerialUSB.println("SDIO_ERR: Unusuable Card,");
            SerialUSB.print("          Check pattern 0x");
            SerialUSB.println(status8.CHECK_PATTERN, HEX);
            return;
        } else {
            SerialUSB.println("SDIO_DBG: Valid check pattern");
        }
        if (status8.VOLTAGE_ACCEPTED != SDIO_VOLTAGE_SUPPLIED) {
            SerialUSB.println("SDIO_ERR: Unusuable Card,");
            SerialUSB.print("          Accepted voltage 0x");
            SerialUSB.println(status8.VOLTAGE_ACCEPTED, DEC);
            return;
        } else {
            SerialUSB.println("SDIO_DBG: Valid supplied voltage");
        }
        CSD.version = CSD_VER_2;
        SerialUSB.println("SDIO_DBG: Interface condition check passed");
    } else if (sdio_get_status(this->sdio_d, SDIO_STA_CCRCFAIL)) {
        return;
    } else {
        SerialUSB.println("SDIO_ERR: Unexpected response status");
        return;
    }
// -------------------------------------------------------------------------
    SerialUSB.println("SDIO_DBG: This is the inquiry ACMD41");
    this->cmd(SD_SEND_OP_COND, //ACMD41: inquiry ACMD41
              0,
              SDIO_RESP_SHRT,
              (uint32*)&this->OCR);
    if (sdio_get_status(this->sdio_d, SDIO_STA_CTIMEOUT)) {
        SerialUSB.println("SDIO_ERR: Not SD Card");
        return;
    } else {
        SerialUSB.print("SDIO_DBG: Volatge window of the card 0x");
        SerialUSB.println((uint16)OCR.VOLTAGE_WINDOW, HEX);
    }
// -------------------------------------------------------------------------
    SerialUSB.println("SDIO_DBG: This is the first ACMD41");
    for(int i = 1; i <= 3; i++) {
        this->getOCR(); //ACMD41: first ACMD41
        if (OCR.BUSY == 1) {
            SerialUSB.println("SDIO_DBG: Card is ready");
            break;
        } else {
            SerialUSB.println("SDIO_DBG: OCR busy");
        }
        delay(1);
    }
    if (OCR.VOLTAGE_WINDOW & SDIO_VALID_VOLTAGE_WINDOW) {
        SerialUSB.println("SDIO_DBG: Valid volatge window");
    } else {
        SerialUSB.println("SDIO_ERR: Unusuable Card");
        return;
    }
    if (OCR.CCS == 0) {
        CSD.capacity = CSD_CAP_SDSC;
    }
// -------------------------------------------------------------------------
    SerialUSB.println("SDIO_DBG: Getting Card Idenfication Number");
    this->cmd(ALL_SEND_CID, //CMD2
              0,
              SDIO_RESP_LONG,
              (uint32*)&this->CID);
    SerialUSB.print("SDIO_DBG: Serial Number is ");
    SerialUSB.println(CID.PSN, DEC);
// -------------------------------------------------------------------------
    SerialUSB.print("SDIO_DBG: Relative address is 0x");
    SerialUSB.println(RCA.RCA, HEX);
    SerialUSB.println("SDIO_DBG: Getting new Relative Address");
    this->cmd(SEND_RELATIVE_ADDR, //CMD3
              0,
              SDIO_RESP_SHRT,
              (uint32*)&this->RCA);
    SerialUSB.print("SDIO_DBG: Relative address is 0x");
    SerialUSB.println(RCA.RCA, HEX);
// -------------------------------------------------------------------------
    /**
    SerialUSB.println("SDIO_DBG: Getting Card Specific Data");
    this->getCSD(); //CMD9
// -------------------------------------------------------------------------
    SerialUSB.println("SDIO_DBG: Getting Sd Configuration Register");
    this->getSCR();
    */
// -------------------------------------------------------------------------  
    SerialUSB.println("SDIO_DBG: Initialization complete");
}

/**
 * @brief Configure clock in clock control register and send command to card
 * @param freq 
 */
void SecureDigitalMemoryCard::clockFreq(SDIOClockFrequency freq) {
    sdio_set_clkcr(this->sdio_d, SDIO_CLKCR_CLKEN | (uint32)freq);
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
    this->cmd(SET_BLOCKLEN,
              (0x1 << size),
              SDIO_RESP_SHRT,
              (uint32*)&status);
    if (status.ERROR == SDIO_CSR_ERROR) {
        SerialUSB.println("SDIO_ERR: Error in SET_BLOCKLEN respsonse");
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
    this->cmd(cmd, 0, SDIO_RESP_NONE, NULL);
}

/**
 * @brief Command (without response) to send to card
 * @param cmd Command index to send
 * @param arg Argument to send
 */
void SecureDigitalMemoryCard::cmd(SDIOCommand cmd, uint32 arg) {
    this->cmd(cmd, arg, SDIO_RESP_NONE, NULL);
}

/**
 * @brief Command (with response) to send to card
 * @param cmd Command index to send
 * @param arg Argument to send
 * @param type Wait for response type
 * @param resp Buffer to store response
 */
void SecureDigitalMemoryCard::cmd(SDIOCommand cmd,
                                  uint32 arg,
                                  SDIORespType type,
                                  uint32 *resp) {
    SerialUSB.print("SDIO_DBG: Sending CMD");
    SerialUSB.println(cmd, DEC);
    //sdio_clock_enable(this->sdio_d);
    while (sdio_is_cmd_act(this->sdio_d)) {
        SerialUSB.println("SDIO_DBG: Wait for last command to finish");
    }
    sdio_clear_interrupt(this->sdio_d, ~SDIO_ICR_RESERVED);
    sdio_cfg_interrupt(this->sdio_d, SDIO_MASK_CMDACTIE |
                       SDIO_MASK_CMDSENTIE | SDIO_MASK_CMDRENDIE |
                       SDIO_MASK_CTIMEOUTIE | SDIO_MASK_CCRCFAILIE);
    sdio_load_arg(this->sdio_d, arg);
    sdio_send_cmd(this->sdio_d,
                  (type << SDIO_CMD_WAITRESP_BIT) | cmd | SDIO_CMD_CPSMEN);
    if (sdio_is_cmd_act(this->sdio_d)) {
        SerialUSB.println("SDIO_DBG: Command active");
    }
    SerialUSB.print("SDIO_DBG: Wait for interrupt... ");
    while (1) {
        if (sdio_get_status(this->sdio_d, SDIO_STA_CTIMEOUT)) {
            SerialUSB.println("Command timeout");
            return;
        } else if (sdio_get_status(this->sdio_d, SDIO_STA_CMDREND)) {
            SerialUSB.println("Response recieved");
            break;
        } else if (sdio_get_status(this->sdio_d, SDIO_STA_CMDSENT)) {
            SerialUSB.println("Command sent");
            return;
        } else if (sdio_get_status(this->sdio_d, SDIO_STA_CCRCFAIL)) {
            switch ((uint8)cmd) {
            case 41: // special response format from ACMD41
                SerialUSB.println("Ignoring CRC for ACMD41");
                sdio_clear_interrupt(this->sdio_d, SDIO_ICR_CCRCFAILC);
                break;
            case 52:
                SerialUSB.println("Ignoring CRC for CMD52");
                sdio_clear_interrupt(this->sdio_d, SDIO_ICR_CCRCFAILC);
                break;
            default:
                SerialUSB.println("Command CRC failure");
                return;
            } //end of switch statement
            break;
        } //end of if statements
    } //end of while statement
    if (sdio_get_cmd(this->sdio_d) == (uint32)cmd)  {
        SerialUSB.print("SDIO_DBG: Response from CMD");
        SerialUSB.println(sdio_get_cmd(this->sdio_d), DEC);
        sdio_get_resp_short(this->sdio_d, resp);
    } else if (sdio_get_cmd(this->sdio_d) == 63) {
        switch ((uint8)cmd) {
        case 41:
            SerialUSB.println("SDIO_DBG: Response from ACMD41");
            sdio_get_resp_short(this->sdio_d, resp);
            break;
        case 2:
            SerialUSB.println("SDIO_DBG: Response from CMD2");
            sdio_get_resp_long(this->sdio_d, resp);
            break;
        case 9:
            SerialUSB.println("SDIO_DBG: Response from CMD9");
            sdio_get_resp_long(this->sdio_d, resp);
            break;
        case 10:
            SerialUSB.println("SDIO_DBG: Response from CMD10");
            sdio_get_resp_long(this->sdio_d, resp);
            break;
        default:
            break;
        }
    } else {
        SerialUSB.print("SDIO_ERR: Command mismatch, CMD");
        SerialUSB.println(sdio_get_cmd(this->sdio_d), DEC);
        return;
    }
}

/**
 * @brief Application Command (without response nor argument) to send to card
 * @param acmd Application Command to send
 */
void SecureDigitalMemoryCard::cmd(SDIOAppCommand acmd) {
    this->cmd(acmd, 0, SDIO_RESP_NONE, NULL);
}

/**
 * @brief Application Command (without response) to send to card
 * @param acmd Command to send
 * @param arg Argument to send
 */
void SecureDigitalMemoryCard::cmd(SDIOAppCommand acmd,
                                   uint32 arg) {
    this->cmd(acmd, arg, SDIO_RESP_NONE, NULL);
}

/**
 * @brief Application Command (with response) to send to card
 * @param acmd Application Command to send
 * @param arg Argument to send
 * @param type Wait for response tag 
 * @param resp Buffer to store response
 */
void SecureDigitalMemoryCard::cmd(SDIOAppCommand acmd,
                                   uint32 arg,
                                   SDIORespType type,
                                   uint32 *resp) {
    csr status55;
    for (uint32 i = 1; i <= 3; i++) {
        this->cmd(APP_CMD,
                  (uint32)RCA.RCA << 16,
                  SDIO_RESP_SHRT,
                  (uint32*)&status55);
        //if (this->check(APP_CMD, 0xFF9FC21) == 0) {
        //    break;
        //}
        if (status55.APP_CMD == SDIO_CSR_DISABLED) {
            SerialUSB.println("SDIO_ERR: AppCommand not enabled, try again");
        } else if (status55.COM_CRC_ERROR == SDIO_CSR_ERROR) {
            SerialUSB.println("SDIO_ERR: CRC error, try again");
        } else {
            break;
        }
    }
    if (status55.APP_CMD == SDIO_CSR_ENABLED) {
        SerialUSB.println("SDIO_DBG: AppCommand enabled");
    } else {
        return;
    }
    if (sdio_get_cmd(this->sdio_d) == APP_CMD) {
        this->cmd((SDIOCommand)acmd,
                  arg,
                  type,
                  resp);
    }
}

/**
 * @brief 
 */
uint32 SecureDigitalMemoryCard::check(SDIOCommand cmd, uint32 mask) {
    SerialUSB.print("SDIO_DBG: Card Response Status for CMD");
    SerialUSB.println((uint8)cmd, DEC);
    uint32 temp = 0;
    sdio_get_resp_short(this->sdio_d, &temp);
    temp &= mask;
    csr status;
    uint32 *statusp = (uint32*)&status;
    *statusp = temp;
    
    uint32 error_count = 0;
    if (status.OUT_OF_RANGE == SDIO_CSR_ERROR) {
        SerialUSB.println("SDIO_ERR: Argument was out of the allowed range");
        error_count++;
    }
    if (status.ADDRESS_ERROR == SDIO_CSR_ERROR) {
        SerialUSB.println("SDIO_ERR: Misaligned address");
        error_count++;
    }
    if (status.BLOCK_LEN_ERROR == SDIO_CSR_ERROR) {
        SerialUSB.println("SDIO_ERR: Block length is not allowed");
        error_count++;
    }
    if (status.ERASE_SEQ_ERROR == SDIO_CSR_ERROR) {
        SerialUSB.println("SDIO_ERR: Erase sequence error");
        error_count++;
    }
    if (status.ERASE_PARAM == SDIO_CSR_ERROR) {
        SerialUSB.println("SDIO_ERR: Invalid selection of write-blocks");
        error_count++;
    }
    if (status.WP_VIOLATION == SDIO_CSR_PROTECTED) {
        SerialUSB.println("SDIO_ERR: Attempt to write to a protected block");
        error_count++;
    }
    if (status.CARD_IS_LOCKED == SDIO_CSR_CARD_LOCKED) {
        SerialUSB.println("SDIO_ERR: Card is locked by the host");
        error_count++;
    }
    if (status.LOCK_UNLOCK_FAILED == SDIO_CSR_ERROR) {
        SerialUSB.println("SDIO_ERR: Sequence or password error");
        error_count++;
    }
    if (status.COM_CRC_ERROR == SDIO_CSR_ERROR) {
        SerialUSB.println("SDIO_ERR: CRC error in previous command");
        error_count++;
    }
    if (status.ILLEGAL_COMMAND == SDIO_CSR_ERROR) {
        SerialUSB.println("SDIO_ERR: Command not legal for the card state");
        error_count++;
    }
    if (status.CARD_ECC_FAILED == SDIO_CSR_ERROR) {
        SerialUSB.println("SDIO_ERR: Error correction failed");
        error_count++;
    }
    if (status.CC_ERROR == SDIO_CSR_ERROR) {
        SerialUSB.println("SDIO_ERR: Internal card controller error");
        error_count++;
    }
    if (status.ERROR == SDIO_CSR_ERROR) {
        SerialUSB.println("SDIO_ERR: A general or unknown error occurred");
        error_count++;
    }
    if (status.CSD_OVERWRITE == SDIO_CSR_ERROR) {
        SerialUSB.println("SDIO_ERR: CSD does not match the card content");
        error_count++;
    }
    if (status.WP_ERASE_SKIP == SDIO_CSR_PROTECTED) {
        SerialUSB.println("SDIO_ERR: Erased only partial block due to WP");
        error_count++;
    }
    if (status.CARD_ECC_DISABLED == SDIO_CSR_ECC_DISABLED) {
        SerialUSB.println("SDIO_ERR: Command executed without internal ECC");
        error_count++;
    }
    if (status.ERASE_RESET == SDIO_CSR_SET) {
        SerialUSB.println("SDIO_ERR: Out of sequence erase command received");
        error_count++;
    }
    if (status.READY_FOR_DATA == SDIO_CSR_NOT_READY) {
        SerialUSB.println("SDIO_ERR: Card not ready for data");
        error_count++;
    }
    if (status.APP_CMD == SDIO_CSR_DISABLED) {
        SerialUSB.println("SDIO_ERR: AppCommand not enabled");
        error_count++;
    }
    if (status.AKE_SEQ_ERROR == SDIO_CSR_ERROR) {
        SerialUSB.println("SDIO_ERR: Authentication sequence process error");
        error_count++;
    }
    //SerialUSB.println("SDIO_DBG: Card response was free of error");
    SerialUSB.print("SDIO_DBG: Card state when receiving command, ");
    switch (status.CURRENT_STATE) {
        case 0:
            SerialUSB.println("IDLE");
            break;
        case 1:
            SerialUSB.println("READY");
            break;
        case 2:
            SerialUSB.println("IDENTIFICATION");
            break;
        case 3:
            SerialUSB.println("STANDBY");
            break;
        case 4:
            SerialUSB.println("TRANSFER");
            break;
        case 5:
            SerialUSB.println("SENDING-DATA");
            break;
        case 6:
            SerialUSB.println("RECEIVING-DATA");
            break;
        case 7:
            SerialUSB.println("PROGRAMMING");
            break;
        case 8:
            SerialUSB.println("DISCONNECT");
            break;
        case 15:
            SerialUSB.println("I/O");
            break;
        default:
            SerialUSB.println("RESERVED");
            break;
    }
    return error_count;
}

/**
 * Card register functions
 */

/**
 * @brief Sends a command to get the Operating Conditions Register
 */
void SecureDigitalMemoryCard::getOCR(void) {
    this->cmd(SD_SEND_OP_COND,
              SDIO_HOST_CAPACITY_SUPPORT | (OCR.VOLTAGE_WINDOW << 8),
              //SDIO_HOST_CAPACITY_SUPPORT | (SDIO_VALID_VOLTAGE_WINDOW << 8),
              SDIO_RESP_SHRT,
              (uint32*)&this->OCR);
}

/**
 * @brief Sends an addressed command to get the Card IDentification number
 */
void SecureDigitalMemoryCard::getCID(void) {
    this->cmd(SEND_CID,
              (uint32)RCA.RCA << 16,
              SDIO_RESP_LONG,
              (uint32*)&this->CID);
}

/**
 * @brief Sends an addressed commmand to get the Card Specific Data 
 */
void SecureDigitalMemoryCard::getCSD(void) {
    this->cmd(SEND_CSD,
              (uint32)RCA.RCA << 16,
              SDIO_RESP_LONG,
              (uint32*)&this->CSD);
}

/**
 * @brief 
 */
void SecureDigitalMemoryCard::getSCR(void) {
    this->cmd(SEND_SCR,
              0,
              SDIO_RESP_SHRT,
              (uint32*)&this->SCR);
}

/**
 * @brief Sends a command to set the Driver Stage Register
 */
void SecureDigitalMemoryCard::setDSR(void) {
    this->cmd(SET_DSR, ((uint32)DSR << 16));
}

/**
 * Data Functions
 */

/**
 * @brief Stop transmission to/from card
 */
void SecureDigitalMemoryCard::stop(void) {
    csr status;
    this->cmd(STOP_TRANSMISSION, 0, SDIO_RESP_SHRT, (uint32*)&status);
    if (status.ERROR != SDIO_CSR_ERROR) {
        //FIXME
    }
}

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
                       SDIO_MASK_RXDAVLIE | SDIO_MASK_RXOVERRIE);
    //uint32 count = 512/4;
    csr status;
    this->cmd(READ_SINGLE_BLOCK, addr,
              SDIO_RESP_SHRT, (uint32*)&status);
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
                       SDIO_MASK_TXDAVLIE | SDIO_MASK_TXUNDERRIE);
    //uint32 count = 512/4;
    csr status;
    this->cmd(WRITE_BLOCK, addr,
              SDIO_RESP_SHRT, (uint32*)&status);
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
*/