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

#define SDIO_DEBUG_ON
#define SDIO_DEBUG SerialUSB

static const uint32 SDIO_HOST_CAPACITY_SUPPORT  = 0x1 << 30;
//static const uint32 SDIO_FAST_BOOT              = 0x1 << 29; //Reserved
//static const uint32 SDIO_SDXC_POWER_CONTROL     = 0x1 << 28; 
//static const uint32 SDIO_SWITCH_1V8_REQUEST     = 0x1 << 24; //Not allowed
static const uint32 SDIO_CHECK_PATTERN          = 0xAA;
static const uint32 SDIO_VOLTAGE_SUPPLIED       = 0x1;
static const uint32 SDIO_VOLTAGE_HOST_SUPPORT   = SDIO_VOLTAGE_SUPPLIED << 8;
static const uint32 SDIO_VALID_VOLTAGE_WINDOW   = 0x3000;
//FIXME temporary, replace these with more general routines
static const uint32 SDIO_DATA_TIMEOUT           = 0xFFFFFFFF;
static const uint32 SDIO_DATA_BLOCKSIZE         = 512;

#if CYCLES_PER_MICROSECOND != 72
/* TODO [0.2.0?] something smarter than this */
#warning "Unexpected clock speed; SDIO frequency calculation will be incorrect"
#endif

/**
 * @brief Constructor
 */
SecureDigitalMemoryCard::SecureDigitalMemoryCard(void) {
    this->sdio_d = SDIO;
    this->RCA.RCA = 0x0;
    this->CSD.version = CSD_VER_UNDEF;
}

/**
 * @brief Configure common startup settings 
 */
void SecureDigitalMemoryCard::begin(void) {
    sdio_set_clkcr(this->sdio_d, SDIO_CLK_INIT); 
    sdio_cfg_gpio();
    sdio_init(this->sdio_d);
    if (sdio_card_powered(this->sdio_d)) {
        sdio_power_off(this->sdio_d);
        delay(1);
    }
    if (sdio_card_detect()) {
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_DBG: Card detected");
        #endif
    } else {
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_ERR: Card not detected");
        #endif
        return;
    }
    sdio_power_on(this->sdio_d);
    #if defined(SDIO_DEBUG_ON)
    SDIO_DEBUG.println("SDIO_DBG: Powered on");
    #endif
    sdio_clock_enable(this->sdio_d);
    delay(1);//Microseconds(185);
    this->idle();
    this->initialization();
    this->identification();
}

/**
 * @brief Reset sdio device
 */
void SecureDigitalMemoryCard::end(void) {
    sdio_reset(this->sdio_d);
    this->RCA.RCA = 0x0;
    this->CSD.version = CSD_VER_UNDEF;
    delay(1);
}

/**
 * @brief Send CMD0 to set card into idle state
 */
void SecureDigitalMemoryCard::idle(void) {
    for (int i =1; i <= 3; i++) {
        this->cmd(GO_IDLE_STATE);
        if (this->ruptFlag == SDIO_FLAG_CMDSENT) {
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_DBG: Card in IDLE state");
            #endif
            return;
        } else {
            delay(1);
        }
    }
    SDIO_DEBUG.println("SDIO_ERR: Card not in IDLE state");
}

/**
 * @brief Card Initialization method
 */
void SecureDigitalMemoryCard::initialization(void) {
    #if defined(SDIO_DEBUG_ON)
    SDIO_DEBUG.println("SDIO_DBG: Initializing card");
    #endif
    icr status8;
    uint32 arg = SDIO_HOST_CAPACITY_SUPPORT;
    for (int i = 1; i <= 3; i++) {
        this->cmd(SEND_IF_COND, //CMD8
                //SDIO_SDXC_POWER_CONTROL |
                  SDIO_VOLTAGE_HOST_SUPPORT | SDIO_CHECK_PATTERN,
                  SDIO_RESP_SHORT,
                  (uint32*)&status8);
        if (this->ruptFlag == SDIO_FLAG_CMDREND) {
            break;
        } else {
            idle();
        }
    }
    switch (this->ruptFlag) {
      case SDIO_FLAG_CMDREND:
        if (status8.CHECK_PATTERN != SDIO_CHECK_PATTERN) {
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.print("SDIO_ERR: Unusuable Card, ");
            SDIO_DEBUG.print("Check pattern 0x");
            SDIO_DEBUG.println(status8.CHECK_PATTERN, HEX);
            #endif
            return; // ASSERT(0);
        } else {
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_DBG: Valid check pattern");
            #endif
        }
        if (status8.VOLTAGE_ACCEPTED != SDIO_VOLTAGE_SUPPLIED) {
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.print("SDIO_ERR: Unusuable Card,");
            SDIO_DEBUG.print("Accepted voltage 0x");
            SDIO_DEBUG.println(status8.VOLTAGE_ACCEPTED, HEX);
            #endif
            return; // ASSERT(0);
        } else {
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_DBG: Valid supplied voltage");
            #endif
        }
        CSD.version = CSD_VER_2;
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_DBG: Interface condition check passed");
        #endif
        break;
      case SDIO_FLAG_CTIMEOUT:
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_DBG: Card does not support CMD8");
        #endif
        // the host should set HCS to 0 if the card returns no response
        arg &= ~SDIO_HOST_CAPACITY_SUPPORT;
        // probably version 1.x memory card
        CSD.version = CSD_VER_1;
        return;
      case SDIO_FLAG_CCRCFAIL:
        return;
      default:
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_ERR: Unexpected response status");
        #endif
        return; // ASSERT(0);
    }
    delay(1);
// -------------------------------------------------------------------------
    #if defined(SDIO_DEBUG_ON)
    SDIO_DEBUG.println("SDIO_DBG: This is the inquiry ACMD41");
    #endif
    this->cmd(SD_SEND_OP_COND, //ACMD41: inquiry ACMD41
              0,
              SDIO_RESP_TYPE7,
              (uint32*)&this->OCR);
    switch (this->ruptFlag) {
      case SDIO_FLAG_CTIMEOUT:
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_ERR: Not SD Card");
        #endif
        return; // ASSERT(0);
      case SDIO_FLAG_CCRCFAIL:
        return;
      default:
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.print("SDIO_DBG: Volatge window of the card 0x");
        SDIO_DEBUG.println((uint16)OCR.VOLTAGE_WINDOW, HEX);
        #endif
        break;
    }
// -------------------------------------------------------------------------
    #if defined(SDIO_DEBUG_ON)
    SDIO_DEBUG.println("SDIO_DBG: This is the first ACMD41");
    #endif
    for(int i = 1; i <= 10; i++) {
        this->getOCR(); //ACMD41: first ACMD41
        if (OCR.BUSY == 1) {
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_DBG: Card is ready");
            #endif
            break;
        } else {
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_DBG: OCR busy");
            #endif
            delay(1);
        }
    }
    if (OCR.BUSY == 0) {
        return;
    } else if (OCR.VOLTAGE_WINDOW & SDIO_VALID_VOLTAGE_WINDOW) {
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_DBG: Valid volatge window");
        #endif
    } else {
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_ERR: Unusuable Card");
        #endif
        return;
    }
    if (OCR.CCS == 0) {
        CSD.capacity = CSD_CAP_SDSC;
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_DBG: Card supports SDSC only");
        #endif
    } else {
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_DBG: Card supports SDHC and SDXC");
        #endif
    }
    SDIO_DEBUG.println("SDIO_DBG: Initialization complete");
}

/**
 * @brief Identify the card with a Relative Card Address
 */
void SecureDigitalMemoryCard::identification(void) {
    #if defined(SDIO_DEBUG_ON)
    SDIO_DEBUG.println("SDIO_DBG: Getting Card Identification Number");
    #endif
    this->cmd(ALL_SEND_CID, //CMD2
              0,
              SDIO_RESP_LONG,
              (uint32*)&this->CID);
    #if defined(SDIO_DEBUG_ON)
    SDIO_DEBUG.print("SDIO_DBG: Card serial number ");
    SDIO_DEBUG.println(CID.PSN, DEC);
// -------------------------------------------------------------------------
    SDIO_DEBUG.println("SDIO_DBG: Getting new Relative Card Address");
    #endif
    this->newRCA();
    #if defined(SDIO_DEBUG_ON)
    SDIO_DEBUG.print("SDIO_DBG: RCA is 0x");
    SDIO_DEBUG.println(RCA.RCA, HEX);
    SDIO_DEBUG.println("SDIO_DBG: Card should now be in STANDBY state"); //FIXME
    #endif
}

/**
 * @brief Configure clock in clock control register and send command to card
 * @param freq 
 */
void SecureDigitalMemoryCard::clockFreq(SDIOClockFrequency freq) {
    //sdio_clock_disable(this->sdio_d);
    sdio_cfg_clkcr(this->sdio_d, SDIO_CLKCR_CLKDIV, (uint32)freq);
    #if defined(SDIO_DEBUG_ON)
    float speed = (CYCLES_PER_MICROSECOND*1000.0)/((float)freq+2.0);
    SDIO_DEBUG.println("SDIO_DBG: Clock speed is ");
    if (speed > 1000) {
        speed /= 1000.0;
        SDIO_DEBUG.print(speed, DEC);
        SDIO_DEBUG.println(" MHz");
    } else {
        SDIO_DEBUG.print(speed, DEC);
        SDIO_DEBUG.println(" kHz");
    }
    #endif
}

/**
 * @brief Change bus width in host and card
 * @param width WIDBUS value to set
 * @note Card bus width can only be changed when card is unlocked
 */
void SecureDigitalMemoryCard::busMode(SDIOBusMode width) {
    csr status;
    this->cmd(SET_BUS_WIDTH, //ACMD6
              width,
              SDIO_RESP_SHORT,
              (uint32*)&status);
    //this->check(0x8FF9FE20);
    switch (width) {
      case SDIO_BUS_1BIT:
        sdio_cfg_clkcr(this->sdio_d, SDIO_CLKCR_WIDBUS, 
                       SDIO_CLKCR_WIDBUS_DEFAULT);
        break;
      case SDIO_BUS_4BIT:
        sdio_cfg_clkcr(this->sdio_d, SDIO_CLKCR_WIDBUS,
                       SDIO_CLKCR_WIDBUS_4WIDE);
        break;
      default:
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_ERR: Unknown bus mode request");
        #endif
        return;
    }
}

/**
 * @brief Set data block size for data commands
 * @param size 
 */
void SecureDigitalMemoryCard::blockSize(SDIOBlockSize size) {
    csr status16;
    uint32 blocksize = (uint8)size;
    if (blocksize > 0xF) {
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_ERR: Invalid block size");
        #endif
        return;
    }
    this->select(this->RCA.RCA);
    this->cmd(SET_BLOCKLEN,
              (0x1 << blocksize),
              SDIO_RESP_SHORT,
              (uint32*)&status16);
    //this->check(0x2FF9FE00);
    if (status16.ERROR == SDIO_CSR_ERROR) {
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_ERR: Error in SET_BLOCKLEN respsonse");
        #endif
        return;
    } else {
        sdio_cfg_dcr(this->sdio_d,
                     SDIO_DCTRL_DBLOCKSIZE,
                     blocksize << SDIO_DCTRL_DBLOCKSIZE_BIT);
    }
}

/**
 * Command and App Command wrapper functions
 */

/**
 * @brief Command (without response nor argument) to send to card
 * @param cmd Command index to send
 */
void SecureDigitalMemoryCard::cmd(SDCommand cmd) {
    this->cmd(cmd, 0, SDIO_RESP_NONE, NULL);
}

/**
 * @brief Command (without response) to send to card
 * @param cmd Command index to send
 * @param arg Argument to send
 */
void SecureDigitalMemoryCard::cmd(SDCommand cmd, uint32 arg) {
    this->cmd(cmd, arg, SDIO_RESP_NONE, NULL);
}

/**
 * @brief Command (with response) to send to card
 * @param cmd Command index to send
 * @param arg Argument to send
 * @param type Wait for response type
 * @param resp Buffer to store response
 */
void SecureDigitalMemoryCard::cmd(SDCommand cmd,
                                  uint32 arg,
                                  SDIORespType type,
                                  uint32 *resp) {
    this->cmd(cmd, arg, type, resp, SDIO_MASK_CMDACTIE |
              SDIO_MASK_CMDSENTIE | SDIO_MASK_CMDRENDIE |
              SDIO_MASK_CTIMEOUTIE | SDIO_MASK_CCRCFAILIE);
}

/**
 * @brief Command (with response) to send to card
 * @param cmd Command index to send
 * @param arg Argument to send
 * @param type Wait for response type
 * @param resp Buffer to store response
 * @param rupt Interrupt to set before command is sent
 */
void SecureDigitalMemoryCard::cmd(SDCommand cmd,
                                  uint32 arg,
                                  SDIORespType type,
                                  uint32 *resp,
                                  uint32 rupt) {
    #if defined(SDIO_DEBUG_ON)
    SDIO_DEBUG.print("SDIO_DBG: Sending CMD");
    SDIO_DEBUG.println(cmd, DEC);
    #endif
    //sdio_clock_enable(this->sdio_d);
    sdio_set_interrupt(this->sdio_d, rupt);
    sdio_load_arg(this->sdio_d, arg);
    switch (type) {
      case SDIO_RESP_SHORT:
      case SDIO_RESP_TYPE1:
      case SDIO_RESP_TYPE3:
      case SDIO_RESP_TYPE6:
      case SDIO_RESP_TYPE7:
        sdio_send_cmd(this->sdio_d,
                      SDIO_CMD_CPSMEN | cmd | SDIO_CMD_WAITRESP_SHORT);
        break;
      case SDIO_RESP_LONG:
      case SDIO_RESP_TYPE2:
        sdio_send_cmd(this->sdio_d,
                      SDIO_CMD_CPSMEN | cmd | SDIO_CMD_WAITRESP_LONG);
        break;
      case SDIO_RESP_NONE:
        sdio_send_cmd(this->sdio_d,
                      SDIO_CMD_CPSMEN | cmd | SDIO_CMD_WAITRESP_NONE);
        break;
      default:
        break;
    }
    if (sdio_is_cmd_act(this->sdio_d)) {
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_DBG: Command active");
        #endif
    }
    #if defined(SDIO_DEBUG_ON)
    SDIO_DEBUG.print("SDIO_DBG: Wait for interrupt... ");
    #endif
    while (1) {
        if (sdio_get_status(this->sdio_d, SDIO_STA_CTIMEOUT)) {
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("Command timeout");
            #endif
            sdio_clear_interrupt(this->sdio_d, SDIO_ICR_CTIMEOUTC);
            this->ruptFlag = SDIO_FLAG_CTIMEOUT;
            return;
        } else if (sdio_get_status(this->sdio_d, SDIO_STA_CMDREND)) {
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("Response recieved");
            #endif
            break;
        } else if (sdio_get_status(this->sdio_d, SDIO_STA_CMDSENT)) {
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("Command sent");
            #endif
            this->ruptFlag = SDIO_FLAG_CMDSENT;
            return;
        } else if (sdio_get_status(this->sdio_d, SDIO_STA_CCRCFAIL)) {
            switch ((uint8)cmd) {
              case 41: // special response format from ACMD41
                #if defined(SDIO_DEBUG_ON)
                SDIO_DEBUG.println("Ignoring CRC for ACMD41");
                #endif
                sdio_clear_interrupt(this->sdio_d, SDIO_ICR_CCRCFAILC);
                break;
              case 52:
                #if defined(SDIO_DEBUG_ON)
                SDIO_DEBUG.println("Ignoring CRC for CMD52");
                #endif
                sdio_clear_interrupt(this->sdio_d, SDIO_ICR_CCRCFAILC);
                break;
              default:
                #if defined(SDIO_DEBUG_ON)
                SDIO_DEBUG.println("Command CRC failure");
                #endif
                sdio_clear_interrupt(this->sdio_d, SDIO_ICR_CCRCFAILC);
                this->ruptFlag = SDIO_FLAG_CCRCFAIL;
                return;
            } //end of switch statement
            break;
        } //end of if statement
    } //end of while statement
    this->ruptFlag = SDIO_FLAG_CMDREND;
    if (sdio_get_cmd(this->sdio_d) == (uint32)cmd) {
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.print("SDIO_DBG: Response from CMD");
        SDIO_DEBUG.println(sdio_get_cmd(this->sdio_d), DEC);
        #endif
        sdio_get_resp_short(this->sdio_d, resp);
        sdio_clear_interrupt(this->sdio_d, SDIO_ICR_CMDRENDC);
    } else if (sdio_get_cmd(this->sdio_d) == 0x3F) { //FIXME
        switch ((uint8)cmd) {
          case 41:
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_DBG: Response from ACMD41");
            #endif
            sdio_get_resp_short(this->sdio_d, resp);
            break;
          case 2:
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_DBG: Response from CMD2");
            #endif
            sdio_get_resp_long(this->sdio_d, resp);
            sdio_clear_interrupt(this->sdio_d, SDIO_ICR_CMDRENDC);
            break;
          case 9:
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_DBG: Response from CMD9");
            #endif
            sdio_get_resp_long(this->sdio_d, resp);
            sdio_clear_interrupt(this->sdio_d, SDIO_ICR_CMDRENDC);
            break;
          case 10:
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_DBG: Response from CMD10");
            #endif
            sdio_get_resp_long(this->sdio_d, resp);
            sdio_clear_interrupt(this->sdio_d, SDIO_ICR_CMDRENDC);
            break;
          default:
            break;
        }
    } else {
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.print("SDIO_ERR: Command mismatch, response from CMD");
        SDIO_DEBUG.println(sdio_get_cmd(this->sdio_d), DEC);
        #endif
        this->ruptFlag = SDIO_FLAG_ERROR;
    }
    sdio_clear_interrupt(this->sdio_d, 
                         SDIO_ICR_CMDSENTC | SDIO_ICR_CMDRENDC |
                         SDIO_ICR_CTIMEOUTC | SDIO_ICR_CCRCFAILC);
}

/**
 * @brief Application Command (without response nor argument) to send to card
 * @param acmd Application Command to send
 */
void SecureDigitalMemoryCard::cmd(SDAppCommand acmd) {
    this->cmd(acmd, 0, SDIO_RESP_NONE, NULL);
}

/**
 * @brief Application Command (without response) to send to card
 * @param acmd Command to send
 * @param arg Argument to send
 */
void SecureDigitalMemoryCard::cmd(SDAppCommand acmd,
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
void SecureDigitalMemoryCard::cmd(SDAppCommand acmd,
                                  uint32 arg,
                                  SDIORespType type,
                                  uint32 *resp) {
    this->cmd(acmd, arg, type, resp, SDIO_MASK_CMDACTIE |
              SDIO_MASK_CMDSENTIE | SDIO_MASK_CMDRENDIE |
              SDIO_MASK_CTIMEOUTIE | SDIO_MASK_CCRCFAILIE);
}

/**
 * @brief Application Command (with response) to send to card
 * @param acmd Application Command to send
 * @param arg Argument to send
 * @param type Wait for response tag 
 * @param resp Buffer to store response
 * @param rupt Interrupt to set before command is sent
 */
void SecureDigitalMemoryCard::cmd(SDAppCommand acmd,
                                  uint32 arg,
                                  SDIORespType type,
                                  uint32 *resp,
                                  uint32 rupt) {
    csr status55;
    for (uint32 i = 1; i <= 3; i++) {
        this->cmd(APP_CMD,
                  (uint32)RCA.RCA << 16,
                  SDIO_RESP_TYPE1,
                  (uint32*)&status55);
        //this->check(0xFF9FC21);
        if (status55.APP_CMD == SDIO_CSR_DISABLED) {
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_DBG: AppCommand not enabled, try again");
            #endif
        } else if (status55.COM_CRC_ERROR == SDIO_CSR_ERROR) {
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_DBG: CRC error, try again");
            #endif
        } else {
            break;
        }
    }
    if (status55.APP_CMD == SDIO_CSR_DISABLED) {
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_DBG: AppCommand not enabled, exiting routine");
        #endif
        return;
    }
    if (sdio_get_cmd(this->sdio_d) == APP_CMD) {
        this->cmd((SDCommand)acmd, arg, type, resp, rupt);
    }
}

/**
 * @brief 
 */
void SecureDigitalMemoryCard::response(SDCommand cmd) {
    switch (cmd) {
      case GO_IDLE_STATE:
        break;
      case ALL_SEND_CID:
      case SEND_CID:
        sdio_get_resp_long(this->sdio_d, (uint32*)&this->CID);
        break;
      case SEND_CSD:
        sdio_get_resp_long(this->sdio_d, (uint32*)&this->CSD);
        break;
      default:
        sdio_get_resp_short(this->sdio_d, (uint32*)&this->CSR);
        break;
    }

}

/**
 * Card register functions
 */

/**
 * @brief Sends a command to get the Operating Conditions Register
 */
void SecureDigitalMemoryCard::newRCA(void) {
    this->cmd(SEND_RELATIVE_ADDR, //CMD3
              0,
              SDIO_RESP_TYPE6,
              (uint32*)&this->RCA);
}

/**
 * @brief Sends a command to get the Operating Conditions Register
 * @note  Only allowed during identification mode
 */
void SecureDigitalMemoryCard::getOCR(void) {
    this->cmd(SD_SEND_OP_COND, //ACMD41
              //SDIO_HOST_CAPACITY_SUPPORT | (OCR.VOLTAGE_WINDOW << 8),
              SDIO_HOST_CAPACITY_SUPPORT | (SDIO_VALID_VOLTAGE_WINDOW << 8),
              SDIO_RESP_TYPE3,
              (uint32*)&this->OCR);
}

/**
 * @brief Sends an addressed command to get the Card IDentification number
 */
void SecureDigitalMemoryCard::getCID(void) {
    this->cmd(SEND_CID, //CMD10
              (uint32)RCA.RCA << 16,
              SDIO_RESP_TYPE2,
              (uint32*)&this->CID);
}

/**
 * @brief Sends an addressed commmand to get the Card Specific Data 
 */
void SecureDigitalMemoryCard::getCSD(void) {
    uint32 *resp = NULL;
    switch (this->CSD.version) {
      case CSD_VER_1:
        resp = (uint32*)&this->CSD.V1;
        break;
      case CSD_VER_2:
        resp = (uint32*)&this->CSD.V2;
        break;
      default:
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_ERR: CSD version undefined");
        #endif
        return;
    }
    this->cmd(SEND_CSD, //CMD9
              (uint32)RCA.RCA << 16,
              SDIO_RESP_TYPE2,
              resp);
    #if defined(SDIO_DEBUG_ON)
    SDIO_DEBUG.println("SDIO_DBG: Card Specific Data received");
    #endif
}

/**
 * @brief 
 */
void SecureDigitalMemoryCard::getSCR(void) {
}

/**
 * @brief 
 */
void SecureDigitalMemoryCard::getSSR(uint32 *buf) {
    this->select(this->RCA.RCA);
    //check for busy signal on dat0 line?
    sdio_set_data_timeout(this->sdio_d, SDIO_DATA_TIMEOUT);
    sdio_set_data_length(this->sdio_d, SDIO_DATA_BLOCKSIZE);
    sdio_set_dcr(this->sdio_d, SDIO_DCTRL_DTDIR | SDIO_DCTRL_DTEN);
    this->cmd(SD_STATUS,
              0,
              SDIO_RESP_SHORT,
              (uint32*)&this->CSR,
              SDIO_MASK_DATAENDIE | SDIO_MASK_STBITERRIE | 
              SDIO_MASK_RXDAVLIE | SDIO_MASK_RXOVERRIE |
              SDIO_MASK_DCRCFAILIE | SDIO_MASK_DTIMEOUTIE |
              SDIO_MASK_CMDSENTIE | SDIO_MASK_CMDRENDIE |
              SDIO_MASK_CTIMEOUTIE | SDIO_MASK_CCRCFAILIE |
              SDIO_MASK_CMDACTIE);
    //this->check(0xFF9FC20);
    switch (this->ruptFlag) { 
      case SDIO_FLAG_CMDREND:
        break;
      case SDIO_FLAG_DTIMEOUT:
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_ERR: Data timeout");
        #endif
        return;
      default:
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_ERR: Unknown response in getSSR");
        #endif
        return;
    }
    if (this->CSR.READY_FOR_DATA == SDIO_CSR_READY) {
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_DBG: Ready to receive data");
        #endif
        while (sdio_get_status(this->sdio_d, SDIO_STA_DATAEND) == 0);
    }
    int rxed = 0;
        if (sdio_get_status(this->sdio_d, SDIO_STA_DTIMEOUT)) {
            sdio_clear_interrupt(this->sdio_d, SDIO_ICR_DTIMEOUTC);
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_ERR: Data timeout");
            #endif
            return;
        } else if (sdio_get_status(this->sdio_d, SDIO_STA_DCRCFAIL)) {
            sdio_clear_interrupt(this->sdio_d, SDIO_ICR_DCRCFAILC);
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_ERR: Data CRC fail");
            #endif
            return;
        } else if (sdio_get_status(this->sdio_d, SDIO_STA_STBITERR)) {
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_ERR: Data start-bit");
            #endif
            return;
        } else if (sdio_get_status(this->sdio_d, SDIO_STA_RXOVERR)) {
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_ERR: Data FIFO overrun");
            #endif
            return;
        } else {
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_DBG: No errors in transfer");
            #endif
        }
    for (int i = 1; i <= 16; i++) {
        buf[rxed++] = sdio_read_data(this->sdio_d);
    }
    #if defined(SDIO_DEBUG_ON)
    SDIO_DEBUG.print("SDIO_DBG: Bytes received ");
    SDIO_DEBUG.println(rxed*4, DEC);
    #endif

}

/**
 * @brief Sends a command to set the Driver Stage Register
 */
void SecureDigitalMemoryCard::setDSR(void) {
    this->cmd(SET_DSR, (uint32)DSR << 16);
}

/**
 * Data Functions
 */

/**
 * @brief Stop transmission to/from card
 */
void SecureDigitalMemoryCard::stop(void) {
    csr status12;
    this->cmd(STOP_TRANSMISSION, //CMD12
              0,
              SDIO_RESP_SHORT, //SDIO_RESP_TYPE1b
              (uint32*)&status12);
    //this->check(0xC6F85E00);
}

/**
 * @brief Read next word from FIFO 
 * @retval Data that was read from FIFO
 */
void SecureDigitalMemoryCard::read(uint32 addr,
                                   uint32 *buf,
                                   uint32 count) {
}

/**
 * @brief Write next word into FIFO
 * @param word Data to write to FIFO
 */
void SecureDigitalMemoryCard::write(uint32 addr, 
                                    const uint32 *buf,
                                    uint32 count) {
}

/**
 * @brief 
 * @param buf Buffer to save data to
 * @param addr Block address to read from
 */
void SecureDigitalMemoryCard::readBlock(uint32 addr, uint32 *buf) {
    //CCS must equal one for block unit addressing
    this->select(this->RCA.RCA);
    //check for busy signal on dat0 line?
    sdio_set_data_timeout(this->sdio_d, SDIO_DATA_TIMEOUT);
    sdio_set_data_length(this->sdio_d, SDIO_DATA_BLOCKSIZE);
    sdio_add_interrupt(this->sdio_d,
                       SDIO_MASK_RXDAVLIE | SDIO_MASK_DBCKENDIE |
                       SDIO_MASK_DATAENDIE | SDIO_MASK_STBITERRIE | 
                       SDIO_MASK_RXFIFOHFIE | SDIO_MASK_RXFIFOFIE |
                       SDIO_MASK_RXFIFOEIE | SDIO_MASK_RXOVERRIE);
    sdio_cfg_dma_rx(this->sdio_d, buf, SDIO_DATA_BLOCKSIZE);
    sdio_set_dcr(this->sdio_d, (0x9 << SDIO_DCTRL_DBLOCKSIZE_BIT) |
                 SDIO_DCTRL_DTDIR | SDIO_DCTRL_DTEN | SDIO_DCTRL_DMAEN);
    csr status17;
    this->cmd(READ_SINGLE_BLOCK,
              addr,
              SDIO_RESP_SHORT,
              (uint32*)&status17);
    //this->check(0xCFF9FE00);
    switch (this->ruptFlag) { 
      case SDIO_FLAG_CMDREND:
        break;
      default:
        #if defined(SDIO_DEBUG_ON)
        SDIO_DEBUG.println("SDIO_ERR: Unknown response in readBlock");
        #endif
        return;
    }
    while (sdio_get_data_count(this->sdio_d) > 0) {
        if (sdio_get_status(this->sdio_d, SDIO_STA_DTIMEOUT)) {
            sdio_clear_interrupt(this->sdio_d, SDIO_ICR_DTIMEOUTC);
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_ERR: Data timeout");
            #endif
            return;
        } else if (sdio_get_status(this->sdio_d, SDIO_STA_DCRCFAIL)) {
            sdio_clear_interrupt(this->sdio_d, SDIO_ICR_DCRCFAILC);
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_ERR: Data CRC fail");
            #endif
            return;
        } else if (sdio_get_status(this->sdio_d, SDIO_STA_STBITERR)) {
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_ERR: Data start-bit");
            #endif
            return;
        } else if (sdio_get_status(this->sdio_d, SDIO_STA_RXOVERR)) {
            #if defined(SDIO_DEBUG_ON)
            SDIO_DEBUG.println("SDIO_ERR: Data FIFO overrun");
            #endif
            return;
        } else if (sdio_get_status(this->sdio_d, SDIO_STA_DBCKEND)) {
            break;
        }
    }
    #if defined(SDIO_DEBUG_ON)
    SDIO_DEBUG.print("SDIO_DBG: Transfer complete");
    #endif
    sdio_clear_interrupt(this->sdio_d, SDIO_ICR_DBCKENDC);
    sdio_dma_disable(this->sdio_d);
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
    sdio_set_interrupt(this->sdio_d, SDIO_MASK_TXFIFOFIE |
                       SDIO_MASK_TXFIFOEIE | SDIO_MASK_TXFIFOHEIE |
                       SDIO_MASK_TXDAVLIE | SDIO_MASK_TXUNDERRIE);
    //uint32 count = 512/4;
    csr status;
    this->cmd(WRITE_BLOCK, addr,
              SDIO_RESP_SHORT, (uint32*)&status);
    if (sdio_get_status(this->sdio_d, SDIO_ICR_CMDRENDC) == 1) {
        //while (sdio_get_fifo_count(this->sdio_d)) 
        for (int i = 0; i < 8; i++) {
            sdio_write_data(this->sdio_d, buf[i]);
        }
    }
}

void SecureDigitalMemoryCard::select(uint16 card) {
    csr status7;
    this->cmd(SELECT_DESELECT_CARD,
              (uint32)card << 16,
              SDIO_RESP_SHORT, //SDIO_RESP_TYPE1b
              (uint32*)&status7);
    //this->check(0xFF9FF00);
}

void SecureDigitalMemoryCard::deselect(void) {
    this->select(0);
}