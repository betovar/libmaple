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
 * @file HardwareSDIO.cpp
 * @author Brian E Tovar <betovar@leaflabs.com>
 * @brief Wirish SD Memory Card implementation
 */

#include <Card/SecureDigital/HardwareSDIO.h>

static const uint32 SDIO_HOST_CAPACITY_SUPPORT  = 0x1 << 30;
//static const uint32 SDIO_FAST_BOOT              = 0x1 << 29; //Reserved
//static const uint32 SDIO_SDXC_POWER_CONTROL     = 0x1 << 28; 
//static const uint32 SDIO_SWITCH_1V8_REQUEST     = 0x1 << 24; //Not allowed
static const uint32 SDIO_CHECK_PATTERN          = 0xAA;
static const uint32 SDIO_VOLTAGE_SUPPLIED       = 0x1;
static const uint32 SDIO_VOLTAGE_HOST_SUPPORT   = SDIO_VOLTAGE_SUPPLIED << 8;
static const uint32 SDIO_VALID_VOLTAGE_WINDOW   = 0x3000; //3.2-3.4 volts
//FIXME temporary, replace these with more general routines

#if CYCLES_PER_MICROSECOND != 72
/* TODO [0.2.0?] something smarter than this */
#warning "Unexpected clock speed; SDIO frequency calculation will be incorrect"
#endif

/**
 * @brief Constructor for Wirish SDIO peripheral support
 */
HardwareSDIO::HardwareSDIO(void) {
    this->sdio_d = SDIO;
    //initialze RCA
    this->RCA.RCA = 0; //zero addresses all cards
    this->RCA.COM_CRC_ERROR = 0;
    this->RCA.ILLEGAL_COMMAND = 0;
    this->RCA.CURRENT_STATE = 0;
    this->RCA.READY_FOR_DATA = 0;
    this->RCA.APP_CMD = 0;
    this->RCA.AKE_SEQ_ERROR = 0;
    //initialze CID
    this->CID.MID = NULL;
    for (uint32 i=0; i<sizeof(this->CID.OID); i++) {
        this->CID.OID[i] = NULL; //OID is a NULL terminated string
    }
    for (uint32 i=0; i<sizeof(this->CID.PNM); i++) {
        this->CID.PNM[i] = NULL; //PNM is a NULL terminated string
    }
    this->CID.PSN = 0;
    this->CID.PRV.N = 0;
    this->CID.PRV.M = 0;
    this->CID.MDT.YEAR = 0;
    this->CID.MDT.MONTH = 0;
    this->CID.CRC = 0;
    //initialze CSD
    this->CSD.CSD_STRUCTURE = 3; //undefined card version
    this->CSD.TAAC = 0;
    this->CSD.NSAC = 0;
    this->CSD.TRAN_SPEED = 0;
    this->CSD.CCC = 0;
    this->CSD.READ_BL_LEN = 0;
    this->CSD.READ_BL_PARTIAL = 0;
    this->CSD.WRITE_BLK_MISALIGN = 0;
    this->CSD.READ_BLK_MISALIGN = 0;
    this->CSD.DSR_IMP = 0;
    this->CSD.C_SIZE = 0;
    this->CSD.VDD_R_CURR_MIN = 0;
    this->CSD.VDD_R_CURR_MAX = 0;
    this->CSD.VDD_W_CURR_MIN = 0;
    this->CSD.VDD_W_CURR_MAX = 0;
    this->CSD.C_SIZE_MULT = 0;
    this->CSD.ERASE_BLK_EN = 0;
    this->CSD.SECTOR_SIZE = 0;
    this->CSD.WP_GRP_SIZE = 0;
    this->CSD.WP_GRP_ENABLE = 0;
    this->CSD.R2W_FACTOR = 0;
    this->CSD.WRITE_BL_LEN = 0;
    this->CSD.WRITE_BL_PARTIAL = 0;
    this->CSD.FILE_FORMAT_GRP = 0;
    this->CSD.COPY = 0;
    this->CSD.PERM_WRITE_PROTECT = 0;
    this->CSD.TMP_WRITE_PROTECT = 0;
    this->CSD.FILE_FORMAT = 0;
    this->CSD.CRC = 0;
    //initialize AppCommand tracker
    this->appCmd = (SDAppCommand)0;
}

/**
 * @brief Configure common startup settings 
 */
void HardwareSDIO::begin(SDIOClockFrequency freq) {
    sdio_set_clkcr(SDIO_CLK_INIT | SDIO_CLKCR_CLKEN);
    sdio_card_detect();
    sdio_init();
    sdio_power_on();
  //sdio_clock_enable();
    delay(10);//Microseconds(185);
    this->idle();
    this->initialization();
    this->identification();
    this->clockFreq(freq);
}

/**
 * @brief General startup settings 
 */
void HardwareSDIO::begin(void) {
    this->begin(SDIO_1_MHZ);
}

/**
 * @brief Reset sdio device
 */
void HardwareSDIO::end(void) {
  //this->command(GO_INACTIVE_STATE);
    sdio_reset();
    this->RCA.RCA = 0x0;
    this->CSD.CSD_STRUCTURE = 3;
    delay(100);
}

/**
 * @brief Send CMD0 to set card into idle state
 */
void HardwareSDIO::idle(void) {
    SDIOInterruptFlag temp = this->responseFlag;
    for (int i=1; i<=3; i++) {
        this->command(GO_IDLE_STATE);
        this->response(GO_IDLE_STATE);
        switch (this->responseFlag) {
          case SDIO_FLAG_CMDSENT:
            //FIXME: check that card is in idle state
            this->responseFlag = temp;
            return;
          default:
            delay(10);
            break;
        }
    }
}

/**
 * @brief Card Initialization method
 */
void HardwareSDIO::initialization(void) {
    uint8 MAX_TRIALS = 3;
    uint32 arg = SDIO_HOST_CAPACITY_SUPPORT;
    for (int i=1; i<=MAX_TRIALS; i++) {
        this->command(SEND_IF_COND, //CMD8
                    //SDIO_SDXC_POWER_CONTROL |
                      SDIO_VOLTAGE_HOST_SUPPORT |
                      SDIO_CHECK_PATTERN);
        this->response(SEND_IF_COND);
        if (this->responseFlag == SDIO_FLAG_CMDREND) {
            if (this->ICR.CHECK_PATTERN != SDIO_CHECK_PATTERN) {
                ASSERT(0);
                return;
            }
            if (this->ICR.VOLTAGE_ACCEPTED != SDIO_VOLTAGE_SUPPLIED) {
                ASSERT(0);
                return;
            }
            // version 2.0 card
            this->CSD.CSD_STRUCTURE = 1;
            break;
        } else if (this->responseFlag == SDIO_FLAG_CTIMEOUT) {
            // the host should set HCS to 0 if the card returns no response
            arg &= ~SDIO_HOST_CAPACITY_SUPPORT;
            // probably version 1.x memory card
            this->CSD.CSD_STRUCTURE = 0;
            break;
        } else if (i == MAX_TRIALS) {
            ASSERT(0);
            return;
        } else {
            idle();
            continue;
        } 
    }
/* --------------------------------------------------------------------------*/
    this->command(SD_SEND_OP_COND); //ACMD41: arg 0 means inquiry ACMD41
    if (this->responseFlag == SDIO_FLAG_CMDREND) {
        this->response(SD_SEND_OP_COND);
    } else if (this->responseFlag == SDIO_FLAG_CTIMEOUT) {
        ASSERT(0);
        return;
    } else {
        ASSERT(0);
        return;
    }
/* --------------------------------------------------------------------------*/
    for (int i=1; i<=5; i++) {
        this->command(SD_SEND_OP_COND, //ACMD41: first ACMD41
                      SDIO_HOST_CAPACITY_SUPPORT | 
                      (SDIO_VALID_VOLTAGE_WINDOW << 8));
        this->response(SD_SEND_OP_COND);
        if (this->OCR.BUSY == 1) {
            //FIXME: add variable that card is ready?
            break;
        } else {
            delay(10*i);
            continue;
        }
    }
    if (OCR.BUSY == 0) {
        ASSERT(0);
        return;
    }
    if (OCR.VOLTAGE_WINDOW & SDIO_VALID_VOLTAGE_WINDOW) {
        // Valid voltage window
    } else {
        ASSERT(0);
        return;
    }
    if (OCR.CCS == 0) {
        CSD.capacity = SD_CAP_SDSC;
        // Card supports SDSC only
    } else {
        CSD.capacity = SD_CAP_SDHC;
        // Card supports SDHC and SDXC
    }
    sdio_set_data_timeout(SDIO_DTIMER_DATATIME); //longest wait time
}

/**
 * @brief Identify the card with a Relative Card Address
 */
void HardwareSDIO::identification(void) {
    this->command(ALL_SEND_CID); //CMD2
    this->response(ALL_SEND_CID);
/* --------------------------------------------------------------------------*/
    this->newRCA();
}

/**
 * @brief Configure clock in clock control register and send command to card
 * @param freq 
 */
void HardwareSDIO::clockFreq(SDIOClockFrequency freq) {
    if (freq > SDIO_CLKCR_CLKDIV) {
        // Invalid clock frequency
        return;
    }
    sdio_clock_disable();
    delay(10);
    sdio_cfg_clkcr(SDIO_CLKCR_CLKDIV, (uint32)freq);
    this->clkFreq = freq;
}

/**
 * @brief Change bus width in host and card
 * @param width WIDBUS value to set
 * @note Card bus width can only be changed when card is unlocked
 */
void HardwareSDIO::busMode(SDIOBusMode width) {
    this->select();
    this->command(SET_BUS_WIDTH, width); //ACMD6
    this->response(SET_BUS_WIDTH);
    //this->check(0x8FF9FE20);
    switch (width) {
      case SDIO_BUS_1BIT:
        sdio_cfg_clkcr(SDIO_CLKCR_WIDBUS, SDIO_CLKCR_WIDBUS_DEFAULT);
        break;
      case SDIO_BUS_4BIT:
        sdio_cfg_clkcr(SDIO_CLKCR_WIDBUS, SDIO_CLKCR_WIDBUS_4WIDE);
        break;
      default:
        // Unsupported bus mode request
        return;
    }
}

/**
 * @brief Sets the data block length for data commands in the host and card
 * @param size block size to set
 * @note This change is reflected in the class variable 'blockSize'
 */
void HardwareSDIO::blockSize(SDIOBlockSize size) {
    if ((size > 0xF) || (size <= 0)) {
        return;
    } else if (size == this->blkSize) {
        return;
    }
    this->select();
    this->command(SET_BLOCKLEN, 0x1 << size);
  //this->check(0x2FF9FE00);
    this->response(SET_BLOCKLEN);
    if (this->CSR.ERROR == SDIO_CSR_ERROR) {
        return;
    } else if (this->CSR.BLOCK_LEN_ERROR == SDIO_CSR_ERROR) {
        return;
    } else if (this->CSR.CARD_IS_LOCKED == SDIO_CSR_CARD_LOCKED) {
        return;
    } else { //FIXME: place more error checking here
    }
    this->blkSize = size;
}

/**
 * Command and App Command wrapper functions
 */

/**
 * @brief Command (without argument) to send to card
 * @param cmd Command index to send
 */
void HardwareSDIO::command(SDCommand cmd) {
    this->command(cmd, 0);
}

/**
 * @brief Command to send to card
 * @param cmd Command index to send
 * @param arg Argument to send
 */
void HardwareSDIO::command(SDCommand cmd, uint32 arg) {
    sdio_clock_enable();
    sdio_enable_interrupt(0x3FFFFF); //almost all interrupts
    sdio_clear_interrupt(~SDIO_ICR_RESERVED);
    sdio_load_arg(arg);
    uint32 cmdreg = (cmd & SDIO_CMD_CMDINDEX) | SDIO_CMD_CPSMEN;
    switch(cmd) {
      case GO_IDLE_STATE:
      case SET_DSR:
      case GO_INACTIVE_STATE:
        cmdreg |= SDIO_CMD_WAITRESP_NONE;
        break;
      case ALL_SEND_CID:
      case SEND_CID:
      case SEND_CSD:
        cmdreg |= SDIO_CMD_WAITRESP_LONG;
        break;
      case READ_SINGLE_BLOCK:
    //case READ_MULTIPLE_BLOCK: //FIXME: add support for other read commands
    //case SEND_WRITE_PROT:
    //case GEN_CMD: //in read mode
        cmdreg |= SDIO_CMD_WAITRESP_SHORT;
        sdio_set_data_length(0x1 << this->blkSize);
        break;
      case WRITE_BLOCK:
    //case WRITE_MULTIPLE_BLOCK:
    //case PROGRAM_CSD:
    //case LOCK_UNLOCK:
    //case GEN_CMD: //in write mode
        cmdreg |= SDIO_CMD_WAITRESP_SHORT;
        sdio_set_data_length(0x1 << this->blkSize);
        break;
      default:
        cmdreg |= SDIO_CMD_WAITRESP_SHORT;
        break;
    }
    delay(10); // wait at least 8 clocks for CPSM to clear
    sdio_send_command(cmdreg);
    if (sdio_check_status(SDIO_STA_CMDACT)) {
    }
}

/**
 * @brief Application Command (without argument) to send to card
 * @param acmd Application Command to send
 */
void HardwareSDIO::command(SDAppCommand acmd) {
    this->command(acmd, 0);
}

/**
 * @brief Application Command to send to card
 * @param acmd Command to send
 * @param arg Argument to send
 */
void HardwareSDIO::command(SDAppCommand acmd, uint32 arg) {
    for (uint32 i=1; i<=4; i++) {
        this->command(APP_CMD, (uint32)this->RCA.RCA << 16);
        this->response(APP_CMD);
      //this->check(0xFF9FC21);
        if (this->CSR.APP_CMD == SDIO_CSR_DISABLED) {
            continue;
        } else if (this->CSR.COM_CRC_ERROR == SDIO_CSR_ERROR) {
            continue;
        } else {
            break;
        }
    }
    if (this->CSR.APP_CMD != SDIO_CSR_ENABLED) {
        return;
    } else if (sdio_get_command() != APP_CMD) {
        return;
    } else {
        this->appCmd = acmd; //keep track of which app command this is
        this->command((SDCommand)acmd, arg);
    }
}

/**
 * @brief Process responses for SD commands
 * @param cmd SDCommand enumeration to process response for
 */
void HardwareSDIO::response(SDCommand cmd) {
    this->wait(cmd);
    uint32 respcmd = sdio_get_command(); //check response matches command
    if (respcmd == (uint32)cmd) {
        this->responseFlag = SDIO_FLAG_CMDREND;
    } else if (respcmd == 0x3F) { //RM0008: pg.576 special case
        switch (cmd) {
          case ALL_SEND_CID:
          case SEND_CID:
          case SEND_CSD:
            this->responseFlag = SDIO_FLAG_CMDREND;
            break;
          default:
            this->responseFlag = SDIO_FLAG_ERROR;
            return;
        }
    } else {
        this->responseFlag = SDIO_FLAG_ERROR;
        return;
    }

    uint32 temp = sdio_get_resp1();
    switch (cmd) {
      case GO_IDLE_STATE: //NO_RESPONSE
      case SET_DSR:
      case GO_INACTIVE_STATE:
        return;
      case SEND_IF_COND: //TYPE_R7
        this->ICR.VOLTAGE_ACCEPTED = (0xF00 & temp) >> 8;
        this->ICR.CHECK_PATTERN = (0xFF & temp);
        break;
      case SEND_RELATIVE_ADDR: //TYPE_R6
        this->convert(&this->RCA);
        break;
      case ALL_SEND_CID: //TYPE_R2
      case SEND_CID:
        this->convert(&this->CID);
        break;
      case SEND_CSD: //TYPE_R2
        this->convert(&this->CSD);
        break;
      case WRITE_BLOCK:
      case SEND_STATUS:
      default: //FIXME assumed all others are TYPE_R1
        this->convert(&this->CSR, temp);
        break;
    }
}

/**
 * @brief Process response for SDAppCommands
 * @param acmd SDAppCommand enumeration to process response for
 */
void HardwareSDIO::response(SDAppCommand acmd) {
    this->wait((SDCommand)acmd);
    this->appCmd = (SDAppCommand)0; //FIXME: is this safe?
    uint32 respcmd = sdio_get_command();
    if (respcmd == (uint32)acmd) {
        this->responseFlag = SDIO_FLAG_CMDREND;
    } else if (respcmd == 0x3F) { //RM0008: pg.576 special case
        switch (acmd) {
          case SD_SEND_OP_COND:
            this->responseFlag = SDIO_FLAG_CMDREND;
            break;
          default:
            this->responseFlag = SDIO_FLAG_ERROR;
            return;
        }
    } else {
        this->responseFlag = SDIO_FLAG_ERROR;
        return;
    }

    if (this->responseFlag != SDIO_FLAG_CMDREND) {
        return;
    }
    uint32 temp = sdio_get_resp1();
    switch (acmd) {
      case SD_SEND_OP_COND: //TYPE_R3
        this->OCR.BUSY = (0x80000000 & temp) >> 31;
        this->OCR.CCS = (0x40000000 & temp) >> 30;
        this->OCR.S18A = (0x1000000 & temp) >> 24;
        this->OCR.VOLTAGE_WINDOW = (0xFFFF00 & temp) >> 8;
        break;
      case SD_STATUS: //TYPE_R1
      case SET_BUS_WIDTH:
      case SEND_NUM_WR_BLOCKS:
      case SET_WR_BLK_ERASE_COUNT:
      case SET_CLR_CARD_DETECT:
      case SEND_SCR:
      default: //FIXME: assumed all others are TYPE_R1
        this->convert(&this->CSR, sdio_get_resp1());
        break;
    }
}

/**
 * @brief Sets up and handles DMA data transfer for AppCommands
 */
void HardwareSDIO::transfer(SDCommand cmd) {
    switch (cmd) {
      case READ_SINGLE_BLOCK:
      case READ_MULTIPLE_BLOCK:
        return;
      case WRITE_BLOCK:
      case WRITE_MULTIPLE_BLOCK:
        break;
    //case SEND_TUNING_BLOCK:
      case PROGRAM_CSD:
      case SEND_WRITE_PROT:
      case LOCK_UNLOCK:
      case GEN_CMD:
    //case SWITCH_FUNC:
        return;
      default:
        return;
    }
    while (!sdio_check_status(SDIO_STA_DBCKEND)) {
        if (sdio_check_status(SDIO_STA_STBITERR | SDIO_STA_DTIMEOUT |
                              SDIO_STA_DCRCFAIL | SDIO_STA_TXUNDERR)) {
            return;
        }
    }
}

/**
 * @brief Sets up and handles polling data transfer for AppCommands
 */
void HardwareSDIO::transfer(SDAppCommand cmd) {
    switch (cmd) {
      case SD_STATUS:
      case SEND_SCR:
        break;
      case SEND_NUM_WR_BLOCKS:
        return;
      default:
        return;
    }

    if (this->CSR.READY_FOR_DATA == SDIO_CSR_READY) {}

    while (1) {
        if (sdio_check_status(SDIO_STA_DTIMEOUT)) {
            sdio_clear_interrupt(SDIO_ICR_DTIMEOUTC);
            return;
        }
        if (sdio_check_status(SDIO_STA_DCRCFAIL)) {
            sdio_clear_interrupt(SDIO_ICR_DCRCFAILC);
            return;
        }
        if (sdio_check_status(SDIO_STA_STBITERR)) {
            return;
        }
        if (sdio_check_status(SDIO_STA_RXOVERR)) {
            return;
        }
    }

    uint32 buf[16]; //max 64-bytes or 512-bits in 32-bit words
    int rxed = 0;
    while (sdio_get_fifo_count() > 0) {
        buf[rxed++] = sdio_read_data();
    }

    switch (cmd) {
      case SD_STATUS:
        break;
      case SEND_SCR:
        this->SCR.SCR_STRUCTURE = (0xF0000000 & buf[0]) >> 28;
        this->SCR.SD_SPEC = (0xF000000 & buf[0]) >> 24;
        this->SCR.DATA_STAT_AFTER_ERASE = (0x800000 & buf[0]) >> 23;
        this->SCR.SD_SECURITY = (0x700000 & buf[0]) >> 20;
        this->SCR.SD_BUS_WIDTHS = (0xF0000 & buf[0]) >> 16;
        this->SCR.SD_SPEC3 = (0x8000 & buf[0]) >> 15;
        this->SCR.EX_SECURITY = (0x7800 & buf[0]) >> 11;
        this->SCR.CMD_SUPPORT = (0x3 & buf[0]);
        break;
      case SEND_NUM_WR_BLOCKS:
      default:
        return;
    }
}

/**
 * @brief 
 */
void HardwareSDIO::wait(SDCommand cmd) {
    while (1) {
        if (sdio_check_status(SDIO_STA_CTIMEOUT)) {
            sdio_clear_interrupt(SDIO_ICR_CTIMEOUTC);
            this->responseFlag = SDIO_FLAG_CTIMEOUT;
            return;
        } else if (sdio_check_status(SDIO_STA_CMDSENT)) {
            sdio_clear_interrupt(SDIO_ICR_CMDSENTC);
            this->responseFlag = SDIO_FLAG_CMDSENT;
            return;
        } else if (sdio_check_status(SDIO_STA_CCRCFAIL)) {
            switch (this->appCmd) {
              case SD_SEND_OP_COND: // special response format for ACMD41
                this->responseFlag = SDIO_FLAG_CMDREND;
                break;
              default:
                this->responseFlag = SDIO_FLAG_CCRCFAIL;
                break;
            }
            sdio_clear_interrupt(SDIO_ICR_CCRCFAILC);
            return;
        } else if (sdio_check_status(SDIO_STA_CMDREND)) {
            switch (cmd) {
              case READ_SINGLE_BLOCK:
              case READ_MULTIPLE_BLOCK:
                sdio_set_dcr((this->blkSize << SDIO_DCTRL_DBLOCKSIZE_BIT) |
                             SDIO_DCTRL_DMAEN | SDIO_DCTRL_DTDIR |
                             SDIO_DCTRL_DTEN);
                this->responseFlag = SDIO_FLAG_CMDREND;
                break;
              case WRITE_BLOCK:
              case WRITE_MULTIPLE_BLOCK:
                sdio_set_dcr((this->blkSize << SDIO_DCTRL_DBLOCKSIZE_BIT) |
                             SDIO_DCTRL_DMAEN |
                             SDIO_DCTRL_DTEN);
                this->responseFlag = SDIO_FLAG_CMDREND;
                break;
              default:
                this->responseFlag = SDIO_FLAG_CMDREND;
                break;
            }
            sdio_clear_interrupt(SDIO_ICR_CMDRENDC);
            return;
        } else {
            this->responseFlag = SDIO_FLAG_ERROR;
            return;
        }
    }
}

/**
 * Card register functions
 */

/**
 * @brief Card publishes a new Relative Card Address (RCA)
 */
void HardwareSDIO::newRCA(void) {
    uint16 temp = this->RCA.RCA;
    for (int i=1; i<=3; i++) {
        this->command(SEND_RELATIVE_ADDR); //CMD3
        this->response(SEND_RELATIVE_ADDR);
        if (this->RCA.RCA == temp) { //FIXME: mmake sure RCA changes
        } else {
            break;
        }
    }
    if (this->RCA.COM_CRC_ERROR == SDIO_CSR_ERROR) {
        ASSERT(0);
        return;
    } else if (this->RCA.ILLEGAL_COMMAND == SDIO_CSR_ERROR) {
        ASSERT(0);
        return;
    } else if (this->RCA.ERROR == SDIO_CSR_ERROR) {
        ASSERT(0);
        return;
/*  } else if (this->RCA.READY_FOR_DATA == SDIO_CSR_NOT_READY) {
        ASSERT(0);
        return;
    } else if (this->RCA.APP_CMD == SDIO_CSR_ENABLED) {
        ASSERT(0);
        return; */
    } else {
        switch (this->RCA.CURRENT_STATE) {
          case SDIO_CSR_STBY:
            break;
          default:
            break;
        }
    }
}

/**
 * @brief Gets the Card Specific Data (CSD)
 */
void HardwareSDIO::getCSD(void) {
    this->command(SEND_CSD, (uint32)RCA.RCA << 16); //CMD9
    this->response(SEND_CSD);
}

/**
 * @brief Gets the Card IDentification number (CID)
 */
void HardwareSDIO::getCID(void) {
    this->command(SEND_CID, (uint32)RCA.RCA << 16); //CMD10
    this->response(SEND_CID);
}

/**
 * @brief Gets the Sd card Configuration Register (SCR)
 * @note Data packet format for Wide Width Data is most significant byte first
 */
void HardwareSDIO::getSCR(void) {
    this->blockSize(SDIO_BKSZ_8);
    sdio_set_data_timeout(SDIO_DTIMER_DATATIME);
    sdio_set_data_length(8); //64-bits or 8-bytes
    sdio_set_dcr((this->blkSize << SDIO_DCTRL_DBLOCKSIZE_BIT) |
                 SDIO_DCTRL_DTDIR);
    this->command(SEND_SCR); //ACMD51
    this->response(SEND_SCR);
  //this->transfer(SEND_SCR);
}

/**
 * @brief Gets Sd Status Register contents (SSR)
 * @param buf Buffer to store register data
 * @note Data packet format for Wide Width Data is most significant byte first
 */
void HardwareSDIO::getSSR(void) {
    sdio_set_data_timeout(SDIO_DTIMER_DATATIME);
    sdio_set_data_length(64); //512-bits or 64-bytes
    this->blockSize(SDIO_BKSZ_64);
  //this->select();
    sdio_set_dcr((this->blkSize << SDIO_DCTRL_DBLOCKSIZE_BIT) |
                 SDIO_DCTRL_DTDIR);
    this->command(SD_STATUS); //ACMD13
  //this->response(SD_STATUS);
  //this->check(0xFF9FC20);
  //this->transfer(SD_STATUS);
}

/**
 * @brief Card sends its status register
 */
void HardwareSDIO::getCSR(void) {
    this->command(SEND_STATUS, this->RCA.RCA << 16);
    this->response(SEND_STATUS);
}


/**
 * @brief Sends a command to set the Driver Stage Register (DSR)
 */
void HardwareSDIO::setDSR(void) {
    this->command(SET_DSR, (uint32)DSR << 16);
}

/**
 * Data Functions
 */

/**
 * @brief Selects the card with the specified Relative Card Address
 * @param card RCA for the card to select
 */
void HardwareSDIO::select(uint16 card) {
    this->command(SELECT_DESELECT_CARD, (uint32)card << 16); //CMD7
    this->response(SELECT_DESELECT_CARD);
  //this->check(0xFF9FF00);
}

/**
 * @brief Selects the card with the memeber Relative Card Address
 */
void HardwareSDIO::select(void) {
    this->select(this->RCA.RCA);
}

/**
 * @brief Deselects all cards, synonymous with select(0)
 */
void HardwareSDIO::deselect(void) {
    this->select(0);
}

/**
 * @brief Converts card status from response register into struct
 */
void HardwareSDIO::convert(csr* TEMP, uint32 temp) {
    TEMP->OUT_OF_RANGE          = (0x80000000 & temp) >> 31;
    TEMP->ADDRESS_ERROR         = (0x40000000 & temp) >> 30;
    TEMP->BLOCK_LEN_ERROR       = (0x20000000 & temp) >> 29;
    TEMP->ERASE_SEQ_ERROR       = (0x10000000 & temp) >> 28;
    TEMP->ERASE_PARAM           =  (0x8000000 & temp) >> 27;
    TEMP->WP_VIOLATION          =  (0x4000000 & temp) >> 26;
    TEMP->CARD_IS_LOCKED        =  (0x2000000 & temp) >> 25;
    TEMP->LOCK_UNLOCK_FAILED    =  (0x1000000 & temp) >> 24;
    TEMP->COM_CRC_ERROR         =   (0x800000 & temp) >> 23;
    TEMP->ILLEGAL_COMMAND       =   (0x400000 & temp) >> 22;
    TEMP->CARD_ECC_FAILED       =   (0x200000 & temp) >> 21;
    TEMP->CC_ERROR              =   (0x100000 & temp) >> 20;
    TEMP->ERROR                 =    (0x80000 & temp) >> 19;
    TEMP->CSD_OVERWRITE         =    (0x10000 & temp) >> 16;
    TEMP->WP_ERASE_SKIP         =     (0x8000 & temp) >> 15;
    TEMP->CARD_ECC_DISABLED     =     (0x4000 & temp) >> 14;
    TEMP->ERASE_RESET           =     (0x2000 & temp) >> 13;
    TEMP->CURRENT_STATE         =     (0x1E00 & temp) >>  9;
    TEMP->READY_FOR_DATA        =      (0x100 & temp) >>  8;
    TEMP->APP_CMD               =       (0x20 & temp) >>  5;
    TEMP->AKE_SEQ_ERROR         =        (0x8 & temp) >>  3;
}

/**
 * @brief Converts card identification number from response registers into struct
 */
void HardwareSDIO::convert(cid* TEMP) {
    uint32 temp = sdio_get_resp1();
    TEMP->MID                   = (0xFF000000 & temp) >> 24;
    TEMP->OID[0]                = (char)((0xFF0000 & temp) >> 16);
    TEMP->OID[1]                = (char)((0xFF00 & temp) >> 8);
    TEMP->PNM[0]                = (char)(0xFF & temp);
    temp = sdio_get_resp2();
    TEMP->PNM[1]                = (char)((0xFF000000 & temp) >> 24);
    TEMP->PNM[2]                = (char)((0xFF0000 & temp) >> 16);
    TEMP->PNM[3]                = (char)((0xFF00 & temp) >> 8);
    TEMP->PNM[4]                = (char)(0xFF & temp);
    temp = sdio_get_resp4();
    TEMP->PSN                   = (0xFF000000 & temp) >> 24;
    TEMP->MDT.YEAR              = (0xFF000 & temp) >> 12;
    TEMP->MDT.MONTH             = (0xF00 & temp) >> 8;
    TEMP->CRC                   = (0xFE & temp) >> 1;
  //TEMP->Always0 = (0x1 & temp);
    temp = sdio_get_resp3();
    TEMP->PRV.N                 = (0xF0000000 & temp) >> 28;
    TEMP->PRV.M                 = (0xF000000 & temp) >> 24;
    TEMP->PSN                  |= (0xFFFFFF & temp) << 8;
}

/**
 * @brief Converts card specific data from response registers into struct
 */
void HardwareSDIO::convert(csd* TEMP) {
    uint32 temp = sdio_get_resp1();
  //TEMP->CSD_STRUCTURE         = (0xC0000000 & temp) >> 30; //FIXME
    TEMP->TAAC                  = (0xFF0000 & temp) >> 16;
    TEMP->NSAC                  = (0xFF00 & temp) >> 8;
    TEMP->TRAN_SPEED            = (0xFF & temp);
    temp = sdio_get_resp3();
    switch (TEMP->CSD_STRUCTURE) { // diff in csd versions
      case 0:
        TEMP->C_SIZE            = (0xC0000000 & temp) >> 30;
        TEMP->VDD_R_CURR_MIN    = (0x38000000 & temp) >> 27;
        TEMP->VDD_R_CURR_MAX    =  (0x7000000 & temp) >> 24;
        TEMP->VDD_W_CURR_MIN    =   (0xE00000 & temp) >> 21;
        TEMP->VDD_W_CURR_MAX    =   (0x1C0000 & temp) >> 18;
        break;
      case 1:
        TEMP->C_SIZE            = (0xFFFF0000 & temp) >> 16;
        break;
      default:
        break;
    }
    TEMP->C_SIZE_MULT           =    (0x38000 & temp) >> 15;
    TEMP->ERASE_BLK_EN          =     (0x4000 & temp) >> 14;
    TEMP->SECTOR_SIZE           =     (0x3F80 & temp) >> 7;
    TEMP->WP_GRP_SIZE           =       (0x7F & temp);
    temp = sdio_get_resp2();
    switch (TEMP->CSD_STRUCTURE) { // diff in csd versions
      case 0:
        TEMP->C_SIZE           |= (0x3FF & temp) << 2;
        break;
      case 1:
        TEMP->C_SIZE           |= (0x3F & temp) << 16;
        break;
      default:
        break;
    }
    TEMP->CCC                   = (0xFFF00000 & temp) >> 20;
    TEMP->READ_BL_LEN           =    (0xF0000 & temp) >> 16;
    TEMP->READ_BL_PARTIAL       =     (0x8000 & temp) >> 15;
    TEMP->WRITE_BLK_MISALIGN    =     (0x4000 & temp) >> 14;
    TEMP->READ_BLK_MISALIGN     =     (0x2000 & temp) >> 13;
    TEMP->DSR_IMP               =     (0x1000 & temp) >> 12;
    temp = sdio_get_resp4();
    TEMP->WP_GRP_ENABLE         = (0x80000000 & temp) >> 31;
    TEMP->R2W_FACTOR            = (0x1C000000 & temp) >> 26;
    TEMP->WRITE_BL_LEN          =  (0x3C00000 & temp) >> 22;
    TEMP->WRITE_BL_PARTIAL      =   (0x200000 & temp) >> 21;
    TEMP->FILE_FORMAT_GRP       =     (0x8000 & temp) >> 15;
    TEMP->COPY                  =     (0x4000 & temp) >> 14;
    TEMP->PERM_WRITE_PROTECT    =     (0x2000 & temp) >> 13;
    TEMP->TMP_WRITE_PROTECT     =     (0x1000 & temp) >> 12;
    TEMP->FILE_FORMAT           =      (0xC00 & temp) >> 10;
    TEMP->CRC                   =       (0xFE & temp) >>  1;
}

/**
 * @brief Converts relative card address from response register into struct
 */
void HardwareSDIO::convert(rca* TEMP) {
    uint32 temp = sdio_get_resp1();
    TEMP->RCA                   = (0xFFFF0000 & temp) >> 16;
    TEMP->COM_CRC_ERROR         =     (0x8000 & temp) >> 15;
    TEMP->ILLEGAL_COMMAND       =     (0x2000 & temp) >> 14;
    TEMP->ERROR                 =     (0x1000 & temp) >> 13;
    TEMP->CURRENT_STATE         =     (0x1E00 & temp) >>  9;
    TEMP->READY_FOR_DATA        =      (0x100 & temp) >>  8;
    TEMP->APP_CMD               =       (0x20 & temp) >>  5;
    TEMP->AKE_SEQ_ERROR         =        (0x8 & temp) >>  3;
}

/**
 * @brief Stops data stream transmission to/from card
 */
void HardwareSDIO::stop(void) {
    this->command(STOP_TRANSMISSION); //CMD12
    //this->check(0xC6F85E00);
    //FIXME: handle DPSM for stopping read or write early
}