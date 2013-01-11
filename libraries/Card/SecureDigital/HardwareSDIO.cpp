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
#include <wirish/wirish_time.h>

#if CYCLES_PER_MICROSECOND != 72
/* TODO [0.2.0?] something smarter than this */
#warning "Unexpected clock speed; SDIO frequency calculation will be incorrect"
#endif

/**
 * @brief Constructor for Wirish SDIO peripheral support
 */
HardwareSDIO::HardwareSDIO(void) {
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
 * @brief General startup setting
 */
void HardwareSDIO::begin(void) {
    this->begin(SDIO_1_MHZ);
}

/**
 * @brief Configure common startup settings
 */
void HardwareSDIO::begin(SDIOClockFrequency freq) {
    sdio_cfg_gpio();
    ASSERT(sdio_card_detect()); //FIXME: use EXTI for checking periodically
    sdio_set_clkcr(SDIO_CLK_INIT);
    clkFreq = SDIO_CLK_INIT;
    sdio_set_data_timeout(SDIO_DTIMER_DATATIME); //longest wait time
    sdio_init();
    sdio_power_on();
    sdio_clock_enable();
    delay_us(200);
/* ---------------------------------------------------------- initialization */
    this->idle();
    this->getICR(5);
    this->command(SD_SEND_OP_COND, 0); //ACMD41: arg 0 means inquiry ACMD41
    this->response(SD_SEND_OP_COND);
    this->getOCR(50); //ACMD41: first ACMD41
/* ---------------------------------------------------------- identification */
    this->command(ALL_SEND_CID, 0); //CMD2
    this->response(ALL_SEND_CID);
    this->newRCA();
    this->clockFreq(freq);
}

/**
 * @brief Reset sdio device
 */
void HardwareSDIO::end(void) {
  //this->command(GO_INACTIVE_STATE);
    sdio_reset();
    this->RCA.RCA = 0x0;
    this->CSD.CSD_STRUCTURE = 3;
    delay_us(10000);
}

/**
 * @brief Send CMD0 to set card into idle state
 */
void HardwareSDIO::idle(void) {
    ASSERT(SDIO->regs->POWER & SDIO_POWER_ON);
    this->command(GO_IDLE_STATE, 0);
    this->response(GO_IDLE_STATE);
}

/**
 * @brief Card Initialization method
 */
void HardwareSDIO::initialization(void) {
    
}

/**
 * @brief Configure clock in clock control register and send command to card
 * @param freq 
 */
void HardwareSDIO::clockFreq(SDIOClockFrequency freq) {
    if (freq > SDIO_CLKCR_CLKDIV) {
        return; // Invalid clock frequency
    }
    sdio_clock_disable();
    delay_us(10);
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
 * @brief Command to send to card
 * @param cmd Command index to send
 * @param arg Argument to send
 */
void HardwareSDIO::command(SDCommand cmd, uint32 arg) {
    sdio_enable_interrupt(0x3FFFFF); //almost all interrupts
    sdio_clear_interrupt(~SDIO_ICR_RESERVED);
    sdio_load_arg(arg);
    sdio_clock_enable();
    uint32 cmdreg = (cmd & SDIO_CMD_CMDINDEX) | SDIO_CMD_CPSMEN;
    switch(cmd) {
      case GO_IDLE_STATE:
      case SET_DSR:
      case GO_INACTIVE_STATE:
        cmdreg |= SDIO_CMD_WAITRESP_NONE;
        break;
      case SEND_CID:
      case SEND_CSD:
      case ALL_SEND_CID:
        cmdreg |= SDIO_CMD_WAITRESP_LONG;
        break;
      case READ_SINGLE_BLOCK:
    //case READ_MULTIPLE_BLOCK:
        cmdreg |= SDIO_CMD_WAITRESP_SHORT;
        sdio_set_data_length(0x1 << this->blkSize);
        break;
      case WRITE_BLOCK:
    //case WRITE_MULTIPLE_BLOCK:
        cmdreg |= SDIO_CMD_WAITRESP_SHORT;
        sdio_set_data_length(0x1 << this->blkSize);
        break;
      default:
      case SEND_IF_COND:
        cmdreg |= SDIO_CMD_WAITRESP_SHORT;
        break;
    }
    sdio_send_command(cmdreg);
    while (SDIO->regs->STA & SDIO_STA_CMDACT) {
        delay_us(100);
    }
    ASSERT(SDIO->regs->STA & 0xC5);
}

/**
 * @brief Application Command to send to card
 * @param acmd Command to send
 * @param arg Argument to send
 */
void HardwareSDIO::command(SDAppCommand acmd, uint32 arg) {
    for (uint32 i=1; i<=10; i++) {
        this->command(APP_CMD, (uint32)this->RCA.RCA << 16);
        if (!(SDIO->regs->STA & SDIO_STA_CMDREND)) {
            continue;
        } else if (SDIO->regs->RESPCMD != APP_CMD) {
            continue;
        } else {
            this->response(APP_CMD); //this->check(0xFF9FC21);
            if (this->CSR.COM_CRC_ERROR != SDIO_CSR_ERROR) {
                continue;
            } else if (this->CSR.APP_CMD == SDIO_CSR_ENABLED) {
                continue;
            } else {
                break; //success                
            }
        }
    }
    this->appCmd = acmd; //keeping track of which app command this is
    this->command((SDCommand)acmd, arg);
    this->appCmd = (SDAppCommand)0; //FIXME: is this safe?
}

/**
 * @brief Process responses for SD commands
 * @param cmd SDCommand enumeration to process response for
 */
void HardwareSDIO::response(SDCommand cmd) {
    switch (cmd) {
      case GO_IDLE_STATE: //NO_RESPONSE
      case SET_DSR:
      case GO_INACTIVE_STATE:
        ASSERT(SDIO->regs->STA & SDIO_STA_CMDSENT);
        return;
      case ALL_SEND_CID:
      case SEND_CID:
      case SEND_CSD:
        ASSERT(SDIO->regs->RESPCMD == 0x3F);//RM0008: pg.576 special case
        break;
      default:
        ASSERT(SDIO->regs->STA & SDIO_STA_CMDREND);
        ASSERT((SDCommand)SDIO->regs->RESPCMD == cmd);
        break;
    }
    switch (cmd) {
      case SEND_IF_COND: //TYPE_R7
        this->ICR.VOLTAGE_ACCEPTED = (0xF00 & SDIO->regs->RESP1) >> 8;
        ASSERT(this->ICR.VOLTAGE_ACCEPTED == SDIO_VOLTAGE_SUPPLIED);
        this->ICR.CHECK_PATTERN    = ( 0xFF & SDIO->regs->RESP1);
        ASSERT(this->ICR.CHECK_PATTERN    == SDIO_CHECK_PATTERN);
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
        this->convert(&this->CSR, SDIO->regs->RESP1);
        break;
    }
}

/**
 * @brief Process response for SDAppCommands
 * @param acmd SDAppCommand enumeration to process response for
 */
void HardwareSDIO::response(SDAppCommand acmd) {
    switch (acmd) {
      case SD_SEND_OP_COND: //TYPE_R3
        ASSERT(SDIO->regs->STA & SDIO_STA_CCRCFAIL);
        sdio_clear_interrupt(SDIO_ICR_CCRCFAILC);
        delay_us(100);
        this->convert(&this->OCR);
        break;
      case SD_STATUS: //TYPE_R1
      case SET_BUS_WIDTH:
      case SEND_NUM_WR_BLOCKS:
      case SET_WR_BLK_ERASE_COUNT:
      case SET_CLR_CARD_DETECT:
      case SEND_SCR:
      default: //FIXME: assumed all others are TYPE_R1
        ASSERT((SDAppCommand)SDIO->regs->RESPCMD == acmd);
        this->convert(&this->CSR, SDIO->regs->RESP1);
        break;
    }
}

/**
 * Card register functions
 */

/**
 * @brief Card sends Interface Condition (ICR) to host
 */
void HardwareSDIO::getICR(uint32 trials) {
    for (uint32 i=1; i<=trials; i++) {
        this->command(SEND_IF_COND, //CMD8
                      SDIO_VOLTAGE_HOST_SUPPORT | SDIO_CHECK_PATTERN);
        if (SDIO->regs->STA & SDIO_STA_CMDREND) {
            this->response(SEND_IF_COND);
            this->CSD.CSD_STRUCTURE = 1; // version 2.0 card
            return;
        } else if (i == trials) {
            //FIXME: the host should set HCS to 0 if CMD8 returns no response
            ASSERT(SDIO->regs->STA & SDIO_STA_CTIMEOUT);
            this->CSD.CSD_STRUCTURE = 0; // version 1.x memory card
            break;
        } else {
            idle();
            continue;
        } 
    }
}

/**
 * @brief Card sends Operating Condition (OCR) to host
 */
void HardwareSDIO::getOCR(uint32 trials) {
    for (uint32 i=1; (i<=trials) || (this->OCR.BUSY != 1); i++) {
        this->command(SD_SEND_OP_COND, //ACMD41
                      SDIO_HOST_CAPACITY_SUPPORT | 
                      (SDIO_VALID_VOLTAGE_WINDOW << 8));
        if (SDIO->regs->STA & SDIO_STA_CCRCFAIL) {
            this->response(SD_SEND_OP_COND);
        }
        delay_us(1000*i); //checks for 1 second total over ~50 trials
        continue;
    }
    ASSERT(this->OCR.BUSY == 1);
    ASSERT(this->OCR.VOLTAGE_WINDOW & SDIO_VALID_VOLTAGE_WINDOW);
    if (this->OCR.CCS == 0) {
        CSD.capacity = SD_CAP_SDSC; // Card supports SDSC only
    } else {
        CSD.capacity = SD_CAP_SDHC; // Card supports SDHC and SDXC
    }
}

/**
 * @brief Card publishes a new Relative Card Address (RCA)
 */
void HardwareSDIO::newRCA(void) {
    uint16 temp = this->RCA.RCA;
    for (int i=1; i<=3; i++) {
        this->command(SEND_RELATIVE_ADDR, 0); //CMD3
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
    this->command(SEND_SCR, 0); //ACMD51
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
    this->command(SD_STATUS, 0); //ACMD13
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
 * @brief Converts operating conditions from response register into struct
 */
void HardwareSDIO::convert(ocr* TEMP) {
    uint32 temp = SDIO->regs->RESP1;
    TEMP->BUSY                  = (0x80000000 & temp) >> 31;
    TEMP->CCS                   = (0x40000000 & temp) >> 30;
    TEMP->S18A                  = ( 0x1000000 & temp) >> 24;
    TEMP->VOLTAGE_WINDOW        = (  0xFFFF00 & temp) >>  8;
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
 * @brief Converts card ident number from response registers into struct
 */
void HardwareSDIO::convert(cid* TEMP) {
    uint32 temp = SDIO->regs->RESP1;
    TEMP->MID                   = (0xFF000000 & temp) >> 24;
    TEMP->OID[0]                = (char)((0xFF0000 & temp) >> 16);
    TEMP->OID[1]                = (char)((0xFF00 & temp) >> 8);
    TEMP->PNM[0]                = (char)(0xFF & temp);
    temp = SDIO->regs->RESP2;
    TEMP->PNM[1]                = (char)((0xFF000000 & temp) >> 24);
    TEMP->PNM[2]                = (char)((0xFF0000 & temp) >> 16);
    TEMP->PNM[3]                = (char)((0xFF00 & temp) >> 8);
    TEMP->PNM[4]                = (char)(0xFF & temp);
    temp = SDIO->regs->RESP4;
    TEMP->PSN                   = (0xFF000000 & temp) >> 24;
    TEMP->MDT.YEAR              = (0xFF000 & temp) >> 12;
    TEMP->MDT.MONTH             = (0xF00 & temp) >> 8;
    TEMP->CRC                   = (0xFE & temp) >> 1;
  //TEMP->Always1               = (0x1 & temp);
    temp = SDIO->regs->RESP3;
    TEMP->PRV.N                 = (0xF0000000 & temp) >> 28;
    TEMP->PRV.M                 = (0xF000000 & temp) >> 24;
    TEMP->PSN                  |= (0xFFFFFF & temp) << 8;
}

/**
 * @brief Converts card specific data from response registers into struct
 */
void HardwareSDIO::convert(csd* TEMP) {
    uint32 temp = SDIO->regs->RESP1;
  //TEMP->CSD_STRUCTURE         = (0xC0000000 & temp) >> 30; //FIXME
    TEMP->TAAC                  = (0xFF0000 & temp) >> 16;
    TEMP->NSAC                  = (0xFF00 & temp) >> 8;
    TEMP->TRAN_SPEED            = (0xFF & temp);
    temp = SDIO->regs->RESP3;
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
    temp = SDIO->regs->RESP2;
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
    temp = SDIO->regs->RESP4;
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
    uint32 temp = SDIO->regs->RESP1;
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
    this->command(STOP_TRANSMISSION, 0); //CMD12
    //this->check(0xC6F85E00);
    //FIXME: handle DPSM for stopping read or write early
}