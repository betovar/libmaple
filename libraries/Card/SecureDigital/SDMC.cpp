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

#define SDIO_DEBUG_ON // to turn debugging off, comment out this line
#ifndef DEBUG_DEVICE
#define DEBUG_DEVICE SerialUSB
#endif

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
HardwareSDIO::HardwareSDIO(sdio_dev*) {
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
    this->CID.MID = 0;
    for (int i=0; i<=2; i++) { //OID is a NULL terminated string
        this->CID.OID[i] = 0;
    }
    for (int i=0; i<=5; i++) { //PNM is a NULL terminated string
        this->CID.PNM[i] = 0;
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
 * @brief General constructor
 */
HardwareSDIO::HardwareSDIO(void) {
    HardwareSDIO(SDIO);
}

/**
 * @brief Configure common startup settings 
 */
void HardwareSDIO::begin(void) {
    sdio_set_clkcr(SDIO_CLK_INIT | SDIO_CLKCR_CLKEN); 
    sdio_cfg_gpio();
    sdio_init();
    if (sdio_card_powered()) {
        sdio_power_off();
        delay(1);
    }
    if (sdio_card_detect()) {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_DBG: Card detected");
        #endif
    } else {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_ERR: Card not detected");
        #endif
        return;
    }
    sdio_power_on();
    #if defined(SDIO_DEBUG_ON)
    DEBUG_DEVICE.println("SDIO_DBG: Powered on");
    #endif
    sdio_clock_enable();
    delay(1);//Microseconds(185);
    this->idle();
    this->initialization();
    this->identification();
    this->getCSD();
}

/**
 * @brief Reset sdio device
 */
void HardwareSDIO::end(void) {
    //this->command(GO_INACTIVE_STATE);
    sdio_reset();
    this->RCA.RCA = 0x0;
    this->CSD.CSD_STRUCTURE = 3;
    delay(1);
}

/**
 * @brief Send CMD0 to set card into idle state
 */
void HardwareSDIO::idle(void) {
    SDIOInterruptFlag temp = this->IRQFlag;
    for (int i=1; i<=3; i++) {
        this->command(GO_IDLE_STATE);
        switch (this->IRQFlag) {
          case SDIO_FLAG_CMDSENT:
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("SDIO_DBG: Card should be in IDLE state");
            #endif
            this->IRQFlag = temp;
            return;
          default:
            delay(1);
            break;
        }
    }
    DEBUG_DEVICE.println("SDIO_ERR: Card not in IDLE state");
}

/**
 * @brief Card Initialization method
 */
void HardwareSDIO::initialization(void) {
    #if defined(SDIO_DEBUG_ON)
    DEBUG_DEVICE.println("SDIO_DBG: Initializing card");
    #endif
    uint32 arg = SDIO_HOST_CAPACITY_SUPPORT;
    for (int i=1; i<=3; i++) {
        this->getICR();
        switch (this->IRQFlag) {
          case SDIO_FLAG_CMDREND:
            i = 1000;
            break;
          case SDIO_FLAG_CTIMEOUT:
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("SDIO_DBG: Card does not support CMD8");
            #endif
            // the host should set HCS to 0 if the card returns no response
            arg &= ~SDIO_HOST_CAPACITY_SUPPORT;
            // probably version 1.x memory card
            this->CSD.CSD_STRUCTURE = 0;
            i = 1000;
            break;
          default:
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("SDIO_ERR: Unexpected response status");
            #endif
            idle();
            break;
        }
    }
    delay(1);
// -------------------------------------------------------------------------
    #if defined(SDIO_DEBUG_ON)
    DEBUG_DEVICE.println("SDIO_DBG: This is the inquiry ACMD41");
    #endif
    this->command(SD_SEND_OP_COND); //ACMD41: arg 0 mean inquiry ACMD41
    switch (this->IRQFlag) {
      case SDIO_FLAG_CTIMEOUT:
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_ERR: Not SD Card");
        #endif
        return; // ASSERT(0);
    //case SDIO_FLAG_CCRCFAIL:
    //  return;
      case SDIO_FLAG_CMDREND:
        this->response(SD_SEND_OP_COND);
      default:
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.print("SDIO_DBG: Volatge window of the card 0x");
        DEBUG_DEVICE.println((uint16)OCR.VOLTAGE_WINDOW, HEX);
        #endif
        break;
    }
// -------------------------------------------------------------------------
    #if defined(SDIO_DEBUG_ON)
    DEBUG_DEVICE.println("SDIO_DBG: This is the first ACMD41");
    #endif
    for (int i=1; i<=10; i++) {
        this->getOCR(); //ACMD41: first ACMD41
        if (this->OCR.BUSY == 1) {
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("SDIO_DBG: Card is ready <-------------");
            #endif
            break;
        } else {
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("SDIO_DBG: OCR busy");
            #endif
            delay(1);
        }
    }
    if (OCR.BUSY == 0) {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_ERR: Card is not ready");
        #endif
        return;
    } else if (OCR.VOLTAGE_WINDOW & SDIO_VALID_VOLTAGE_WINDOW) {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_DBG: Valid volatge window");
        #endif
    } else {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_ERR: Unusuable Card");
        #endif
        return;
    }
    if (OCR.CCS == 0) {
        CSD.capacity = SD_CAP_SDSC;
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_DBG: Card supports SDSC only");
        #endif
    } else {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_DBG: Card supports SDHC and SDXC");
        #endif
    }
    DEBUG_DEVICE.println("SDIO_DBG: Initialization complete");
}

/**
 * @brief Identify the card with a Relative Card Address
 */
void HardwareSDIO::identification(void) {
    #if defined(SDIO_DEBUG_ON)
    DEBUG_DEVICE.println("SDIO_DBG: Getting Card Identification Number");
    #endif
    this->command(ALL_SEND_CID); //CMD2
    this->response(ALL_SEND_CID);
    #if defined(SDIO_DEBUG_ON)
    DEBUG_DEVICE.print("SDIO_DBG: Manufaturer ID ");
    DEBUG_DEVICE.println(CID.MID, DEC);
    DEBUG_DEVICE.print("SDIO_DBG: Application ID ");
    DEBUG_DEVICE.println(this->CID.OID);
    DEBUG_DEVICE.print("SDIO_DBG: Product name ");
    DEBUG_DEVICE.println(this->CID.PNM);
    DEBUG_DEVICE.print("SDIO_DBG: Product revision ");
    DEBUG_DEVICE.print(CID.PRV.N, DEC);
    DEBUG_DEVICE.print(".");
    DEBUG_DEVICE.println(CID.PRV.M, DEC);
    DEBUG_DEVICE.print("SDIO_DBG: Serial number 0x");
    DEBUG_DEVICE.println(CID.PSN, HEX);
    DEBUG_DEVICE.print("SDIO_DBG: Manufacture date ");
    DEBUG_DEVICE.print(CID.MDT.MONTH, DEC);
    DEBUG_DEVICE.print("/");
    DEBUG_DEVICE.println(CID.MDT.YEAR+2000, DEC);
// -------------------------------------------------------------------------
    DEBUG_DEVICE.println("SDIO_DBG: Getting new Relative Card Address");
    #endif
    this->newRCA();
    #if defined(SDIO_DEBUG_ON)
    DEBUG_DEVICE.println("SDIO_DBG: Card should now be in STANDBY state");
    #endif
}

/**
 * @brief Configure clock in clock control register and send command to card
 * @param freq 
 */
void HardwareSDIO::clockFreq(SDIOClockFrequency freq) {
  //sdio_clock_disable();
    sdio_cfg_clkcr(SDIO_CLKCR_CLKDIV, (uint32)freq);
    #if defined(SDIO_DEBUG_ON)
    float speed = (CYCLES_PER_MICROSECOND*1000.0)/((float)freq+2.0);
    DEBUG_DEVICE.println("SDIO_DBG: Clock speed is ");
    if (speed > 1000.0) {
        speed /= 1000.0;
        DEBUG_DEVICE.print(speed, DEC);
        DEBUG_DEVICE.println(" MHz");
    } else {
        DEBUG_DEVICE.print(speed, DEC);
        DEBUG_DEVICE.println(" kHz");
    }
    #endif
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
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_ERR: Unknown bus mode request");
        #endif
        return;
    }
}

/**
 * @brief Set data block size for data commands
 * @param size 
 */
void HardwareSDIO::blockLength(SDIOBlockSize size) {
    if ((size > 0xF) || (size <= 0)) {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_ERR: Invalid block size");
        #endif
        return;
    }
    #if defined(SDIO_DEBUG_ON)
    DEBUG_DEVICE.println("SDIO_DBG: Setting block size");
    #endif
    this->select();
    this->command(SET_BLOCKLEN, 0x1 << size);
    //this->check(0x2FF9FE00);
    this->response(SET_BLOCKLEN);
    if (this->CSR.ERROR == SDIO_CSR_ERROR) {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_ERR: Generic Error in SET_BLOCKLEN");
        #endif
        return;
    } else if (this->CSR.BLOCK_LEN_ERROR == SDIO_CSR_ERROR) {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_ERR: Block Length Error in SET_BLOCKLEN");
        #endif
        return;
    } else if (this->CSR.CARD_IS_LOCKED == SDIO_CSR_CARD_LOCKED) {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_ERR: Locked Card in SET_BLOCKLEN");
        #endif
        return;
    } else {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_DBG: SET_BLOCKLEN completed without error");
        #endif
        this->blockSize = size;
    }
}

/**
 * Command and App Command wrapper functions
 */

/**
 * @brief Command (without response nor argument) to send to card
 * @param cmd Command index to send
 */
void HardwareSDIO::command(SDCommand cmd) {
    this->command(cmd, 0);
}

/**
 * @brief Command (without response) to send to card
 * @param cmd Command index to send
 * @param arg Argument to send
 */
void HardwareSDIO::command(SDCommand cmd, uint32 arg) {
    sdio_clock_enable();
    #if defined(SDIO_DEBUG_ON)
    DEBUG_DEVICE.print("SDIO_DBG: Sending CMD");
    DEBUG_DEVICE.println(cmd, DEC);
    #endif
    sdio_set_interrupt(0x3FFFFF); //almost all interrupts
    sdio_load_arg(arg);
    uint32 cmdreg = cmd | SDIO_CMD_CPSMEN; // | SDIO_CMD_WAITINT;

    switch(cmd) { //set response type and data modes
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
      case READ_MULTIPLE_BLOCK: //FIXME
      //cmdreg |= SDIO_CMD_WAITPEND;
        cmdreg |= SDIO_CMD_WAITRESP_SHORT;
        sdio_set_data_timeout(SDIO_DTIMER_DATATIME);
        sdio_set_data_length(0x1 << this->blockSize);
        sdio_set_dcr((this->blockSize << SDIO_DCTRL_DBLOCKSIZE_BIT) |
                     SDIO_DCTRL_DMAEN | SDIO_DCTRL_DTDIR);
        break;
      case WRITE_BLOCK:
      case WRITE_MULTIPLE_BLOCK:
      //cmdreg |= SDIO_CMD_WAITPEND; // wait for data
        cmdreg |= SDIO_CMD_WAITRESP_SHORT;
        sdio_set_data_timeout(SDIO_DTIMER_DATATIME); //longest wait time
        sdio_set_data_length(0x1 << this->blockSize);
        sdio_set_dcr(this->blockSize << SDIO_DCTRL_DBLOCKSIZE_BIT |
                     SDIO_DCTRL_DMAEN);
        break;
      default:
        cmdreg |= SDIO_CMD_WAITRESP_SHORT;
        break;
    }
    sdio_send_command(cmdreg);

    if (sdio_is_cmd_act()) {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_DBG: Command active");
        #endif
    } else {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_ERR: CPSM never went active");
        #endif
    }
    #if defined(SDIO_DEBUG_ON)
    DEBUG_DEVICE.print("SDIO_DBG: Wait for interrupt... ");
    #endif
    while (!SDIO->irq_fired) { //wait for interrupt
    }
    SDIO->irq_fired = 0;
    #if defined(SDIO_DEBUG_ON)
    DEBUG_DEVICE.print("IRQ fired, ");
    #endif
    if (sdio_check_status(SDIO_STA_CTIMEOUT)) {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("Response timeout");
        #endif
        sdio_clear_interrupt(SDIO_ICR_CTIMEOUTC);
        this->IRQFlag = SDIO_FLAG_CTIMEOUT;
        return;
    } else if (sdio_check_status(SDIO_STA_CMDSENT)) {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("Command sent");
        #endif
        sdio_clear_interrupt(SDIO_ICR_CMDSENTC);
        this->IRQFlag = SDIO_FLAG_CMDSENT;
        return;
    } else if (sdio_check_status(SDIO_STA_CCRCFAIL)) {
        switch (this->appCmd) {
            /*
          case 0:
            switch (cmd) {
              case IO_RW_DIRECT:
                #if defined(SDIO_DEBUG_ON)
                DEBUG_DEVICE.println("Ignoring CRC for CMD52");
                #endif
                break;
              default:
                break;
            }
            break;
            */
          case SD_SEND_OP_COND: // special response format for ACMD41
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("Ignoring CRC for ACMD41");
            #endif
            break;
          default:
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("Command CRC failure");
            #endif
            this->IRQFlag = SDIO_FLAG_CCRCFAIL;
            return;
        }
        sdio_clear_interrupt(SDIO_ICR_CCRCFAILC);
        this->IRQFlag = SDIO_FLAG_CMDREND;
        delay(1);
    } else if (sdio_check_status(SDIO_STA_CMDREND)) {
        switch (cmd) {
          case READ_SINGLE_BLOCK:
          case READ_MULTIPLE_BLOCK:
            break;
          case WRITE_BLOCK:
          case WRITE_MULTIPLE_BLOCK:
            //sdio_cfg_dma_tx(dst, 0x1 << this->blockSize);
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("Response received, data transfer starting");
            #endif
            sdio_dt_enable();
            break;
          default:
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("Response received");
            #endif
            break;
        }
        sdio_clear_interrupt(SDIO_ICR_CMDRENDC);
        this->IRQFlag = SDIO_FLAG_CMDREND;
    } else {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("Unexpected interrupt fired");
        #endif
        this->IRQFlag = SDIO_FLAG_ERROR;
        return;
    }

    uint8 respcmd = sdio_get_command(); //check response matches command
    if (respcmd == (uint32)cmd) {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.print("SDIO_DBG: Response from CMD");
        DEBUG_DEVICE.println(respcmd, DEC);
        #endif
    } else if (respcmd == 0x3F) { //RM0008: pg.576 special case
        if (this->appCmd) {
            switch (this->appCmd) {
              case SD_SEND_OP_COND:
                #if defined(SDIO_DEBUG_ON)
                DEBUG_DEVICE.println("SDIO_DBG: Response from ACMD41");
                #endif
                break;
              default:
                #if defined(SDIO_DEBUG_ON)
                DEBUG_DEVICE.println("SDIO_ERR: Unexpected response command");
                this->IRQFlag = SDIO_FLAG_ERROR;
                return;
                #endif
              break;
            }
        } else {
            switch (cmd) {
              case ALL_SEND_CID:
              case SEND_CID:
              case SEND_CSD:
                #if defined(SDIO_DEBUG_ON)
                DEBUG_DEVICE.print("SDIO_DBG: Response from CMD");
                DEBUG_DEVICE.println(cmd, DEC);
                #endif
                break;
              default:
                #if defined(SDIO_DEBUG_ON)
                DEBUG_DEVICE.println("SDIO_ERR: Unexpected response command");
                this->IRQFlag = SDIO_FLAG_ERROR;
                return;
                #endif
            }
        }
    } else {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.print("SDIO_ERR: Command mismatch, response from CMD");
        DEBUG_DEVICE.println(respcmd, DEC);
        #endif
        this->IRQFlag = SDIO_FLAG_ERROR;
        return;
    }
}

/**
 * @brief Application Command (without response nor argument) to send to card
 * @param acmd Application Command to send
 */
void HardwareSDIO::command(SDAppCommand acmd) {
    this->command(acmd, 0);
}

/**
 * @brief Application Command (without response) to send to card
 * @param acmd Command to send
 * @param arg Argument to send
 */
void HardwareSDIO::command(SDAppCommand acmd, uint32 arg) {
    for (uint32 i=1; i<=3; i++) {
        this->command(APP_CMD, (uint32)this->RCA.RCA << 16);
        this->response(APP_CMD);
        //this->check(0xFF9FC21);
        if (this->CSR.APP_CMD == SDIO_CSR_DISABLED) {
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("SDIO_DBG: AppCommand not enabled, try again");
            #endif
        } else if (this->CSR.COM_CRC_ERROR == SDIO_CSR_ERROR) {
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("SDIO_DBG: CRC error, try again");
            #endif
        } else {
            break;
        }
    }
    if (this->CSR.APP_CMD == SDIO_CSR_DISABLED) {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_DBG: AppCommand not enabled, exiting routine");
        #endif
        return;
    }
    if (sdio_get_command() == APP_CMD) {
        this->appCmd = acmd; //keep track of which app command this is
        this->command((SDCommand)acmd, arg);
        this->appCmd = (SDAppCommand)0;
    }
}

/**
 * @brief 
 */
void HardwareSDIO::response(SDCommand cmd) {
    uint32 temp;
    switch (this->IRQFlag) {
      case SDIO_FLAG_CMDREND:
        break;
      default:
        return;
    }
    switch (cmd) {
      case GO_IDLE_STATE: //NO_RESPONSE
      case SET_DSR:
      case GO_INACTIVE_STATE:
        return;
      case SEND_IF_COND: //TYPE_R7
        temp = sdio_get_resp(1);
        this->ICR.VOLTAGE_ACCEPTED = (0xF00 & temp) >> 8;
        this->ICR.CHECK_PATTERN = (0xFF & temp);
        break;
      case SEND_RELATIVE_ADDR: //TYPE_R6
        temp = sdio_get_resp(1);
        this->RCA.RCA = (0xFFFF0000 & temp) >> 16;
        this->RCA.COM_CRC_ERROR = (0x8000 & temp) >> 15;
        this->RCA.ILLEGAL_COMMAND = (0x2000 & temp) >> 14;
        this->RCA.ERROR = (0x1000 & temp) >> 13;
        this->RCA.CURRENT_STATE = (0x1E00 & temp) >> 9;
        this->RCA.READY_FOR_DATA = (0x100 & temp) >> 8;
        this->RCA.APP_CMD = (0x20 & temp) >> 5;
        this->RCA.AKE_SEQ_ERROR = (0x8 & temp) >> 3;
        break;
      case ALL_SEND_CID: //TYPE_R2
      case SEND_CID:
        temp = sdio_get_resp(1);
        this->CID.MID = (0xFF000000 & temp) >> 24;
        this->CID.OID[0] = (char)((0xFF0000 & temp) >> 16);
        this->CID.OID[1] = (char)((0xFF00 & temp) >> 8);
        this->CID.PNM[0] = (char)(0xFF & temp);
        temp = sdio_get_resp(2);
        this->CID.PNM[1] = (char)((0xFF000000 & temp) >> 24);
        this->CID.PNM[2] = (char)((0xFF0000 & temp) >> 16);
        this->CID.PNM[3] = (char)((0xFF00 & temp) >> 8);
        this->CID.PNM[4] = (char)(0xFF & temp);
        temp = sdio_get_resp(4);
        this->CID.PSN = (0xFF000000 & temp) >> 24;
        this->CID.MDT.YEAR = (0xFF000 & temp) >> 12;
        this->CID.MDT.MONTH = (0xF00 & temp) >> 8;
        this->CID.CRC = (0xFE & temp) >> 1;
      //this->CID->Always0 = (0x1 & temp);
        temp = sdio_get_resp(3);
        this->CID.PRV.N = (0xF0000000 & temp) >> 28;
        this->CID.PRV.M = (0xF000000 & temp) >> 24;
        this->CID.PSN |= (0xFFFFFF & temp) << 8;
        break;
      case SEND_CSD: //TYPE_R2
        temp = sdio_get_resp(1);
        //this->CSD.CSD_STRUCTURE = (0xC0000000 & temp) >> 30;
        this->CSD.TAAC = (0xFF0000 & temp) >> 16;
        this->CSD.NSAC = (0xFF00 & temp) >> 8;
        this->CSD.TRAN_SPEED = (0xFF & temp);
        temp = sdio_get_resp(3);
        switch (this->CSD.CSD_STRUCTURE) { // only diff in csd versions
          case 0:
            this->CSD.C_SIZE = (0xC0000000 & temp) >> 30;
            this->CSD.VDD_R_CURR_MIN = (0x38000000 & temp) >> 27;
            this->CSD.VDD_R_CURR_MAX = (0x7000000 & temp) >> 24;
            this->CSD.VDD_W_CURR_MIN = (0xE00000 & temp) >> 21;
            this->CSD.VDD_W_CURR_MAX = (0x1C0000 & temp) >> 18;
            break;
          case 1:
            this->CSD.C_SIZE = (0xFFFF0000 & temp) >> 16;
            break;
          default:
            break;
        }
        this->CSD.C_SIZE_MULT = (0x38000 & temp) >> 15;
        this->CSD.ERASE_BLK_EN = (0x4000 & temp) >> 14;
        this->CSD.SECTOR_SIZE = (0x3F80 & temp) >> 7;
        this->CSD.WP_GRP_SIZE = (0x7F & temp);
        temp = sdio_get_resp(2);
        switch (this->CSD.CSD_STRUCTURE) { // only diff in csd versions
          case 0:
            this->CSD.C_SIZE |= (0x3FF & temp) << 2;
            break;
          case 1:
            this->CSD.C_SIZE |= (0x3F & temp) << 16;
            break;
          default:
            break;
        }
        this->CSD.CCC = (0xFFF00000 & temp) >> 20;
        this->CSD.READ_BL_LEN = (0xF0000 & temp) >> 16;
        this->CSD.READ_BL_PARTIAL = (0x8000 & temp) >> 15;
        this->CSD.WRITE_BLK_MISALIGN = (0x4000 & temp) >> 14;
        this->CSD.READ_BLK_MISALIGN = (0x2000 & temp) >> 13;
        this->CSD.DSR_IMP = (0x1000 & temp) >> 12;
        temp = sdio_get_resp(4);
        this->CSD.WP_GRP_ENABLE = (0x80000000 & temp) >> 31;
        this->CSD.R2W_FACTOR = (0x1C000000 & temp) >> 26;
        this->CSD.WRITE_BL_LEN = (0x3C00000 & temp) >> 22;
        this->CSD.WRITE_BL_PARTIAL = (0x200000 & temp) >> 21;
        this->CSD.FILE_FORMAT_GRP = (0x8000 & temp) >> 15;
        this->CSD.COPY = (0x4000 & temp) >> 14;
        this->CSD.PERM_WRITE_PROTECT = (0x2000 & temp) >> 13;
        this->CSD.TMP_WRITE_PROTECT = (0x1000 & temp) >> 12;
        this->CSD.FILE_FORMAT = (0xC00 & temp) >> 10;
        this->CSD.CRC = (0xFE & temp) >> 1;
        break;
      case SEND_STATUS:
      default: //FIXME assumed all others are TYPE_R1
        temp = sdio_get_resp(1);
        this->CSR.OUT_OF_RANGE = (0x80000000 & temp) >> 31;
        this->CSR.ADDRESS_ERROR = (0x40000000 & temp) >> 30;
        this->CSR.BLOCK_LEN_ERROR = (0x20000000 & temp) >> 29;
        this->CSR.ERASE_SEQ_ERROR = (0x10000000 & temp) >> 28;
        this->CSR.ERASE_PARAM = (0x8000000 & temp) >> 27;
        this->CSR.WP_VIOLATION = (0x4000000 & temp) >> 26;
        this->CSR.CARD_IS_LOCKED = (0x2000000 & temp) >> 25;
        this->CSR.LOCK_UNLOCK_FAILED = (0x1000000 & temp) >> 24;
        this->CSR.COM_CRC_ERROR = (0x800000 & temp) >> 23;
        this->CSR.ILLEGAL_COMMAND = (0x400000 & temp) >> 22;
        this->CSR.CARD_ECC_FAILED = (0x200000 & temp) >> 21;
        this->CSR.CC_ERROR = (0x100000 & temp) >> 20;
        this->CSR.ERROR = (0x80000 & temp) >> 19;
        this->CSR.CSD_OVERWRITE = (0x10000 & temp) >> 16;
        this->CSR.WP_ERASE_SKIP = (0x8000 & temp) >> 15;
        this->CSR.CARD_ECC_DISABLED = (0x4000 & temp) >> 14;
        this->CSR.ERASE_RESET = (0x2000 & temp) >> 13;
        this->CSR.CURRENT_STATE = (0x1E00 & temp) >> 9;
        this->CSR.READY_FOR_DATA = (0x100 & temp) >> 8;
        this->CSR.APP_CMD = (0x20 & temp) >> 5;
        this->CSR.AKE_SEQ_ERROR = (0x8 & temp) >> 3;
        break;
    }
}

/**
 * @brief 
 */
void HardwareSDIO::response(SDAppCommand cmd) {
    uint32 temp;
    switch (this->IRQFlag) {
      case SDIO_FLAG_CMDREND:
        break;
      default:
        return;
    }
    switch (cmd) {
      case SD_SEND_OP_COND: //TYPE_R3
        temp = sdio_get_resp(1);
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
      default: //FIXME assumed all others are TYPE_R1
        temp = sdio_get_resp(1);
        this->CSR.OUT_OF_RANGE = (0x80000000 & temp) >> 31;
        this->CSR.ADDRESS_ERROR = (0x40000000 & temp) >> 30;
        this->CSR.BLOCK_LEN_ERROR = (0x20000000 & temp) >> 29;
        this->CSR.ERASE_SEQ_ERROR = (0x10000000 & temp) >> 28;
        this->CSR.ERASE_PARAM = (0x8000000 & temp) >> 27;
        this->CSR.WP_VIOLATION = (0x4000000 & temp) >> 26;
        this->CSR.CARD_IS_LOCKED = (0x2000000 & temp) >> 25;
        this->CSR.LOCK_UNLOCK_FAILED = (0x1000000 & temp) >> 24;
        this->CSR.COM_CRC_ERROR = (0x800000 & temp) >> 23;
        this->CSR.ILLEGAL_COMMAND = (0x400000 & temp) >> 22;
        this->CSR.CARD_ECC_FAILED = (0x200000 & temp) >> 21;
        this->CSR.CC_ERROR = (0x100000 & temp) >> 20;
        this->CSR.ERROR = (0x80000 & temp) >> 19;
        this->CSR.CSD_OVERWRITE = (0x10000 & temp) >> 16;
        this->CSR.WP_ERASE_SKIP = (0x8000 & temp) >> 15;
        this->CSR.CARD_ECC_DISABLED = (0x4000 & temp) >> 14;
        this->CSR.ERASE_RESET = (0x2000 & temp) >> 13;
        this->CSR.CURRENT_STATE = (0x1E00 & temp) >> 9;
        this->CSR.READY_FOR_DATA = (0x100 & temp) >> 8;
        this->CSR.APP_CMD = (0x20 & temp) >> 5;
        this->CSR.AKE_SEQ_ERROR = (0x8 & temp) >> 3;
        break;
    }
}

/**
 * @brief Sets up and handles polling data transfer for AppCommands
 */
void HardwareSDIO::transfer(SDCommand cmd) {
  //uint32 *buf = NULL;
    switch (cmd) {
      case READ_SINGLE_BLOCK:
      case READ_MULTIPLE_BLOCK:
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_DBG: DMA_RX not yet supported");
        #endif
        return;
      case WRITE_BLOCK:
      case WRITE_MULTIPLE_BLOCK:
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_DBG: DMA_TX not yet supported");
        #endif
        return;
    //case SEND_TUNING_BLOCK:
      case PROGRAM_CSD:
      case SEND_WRITE_PROT:
      case LOCK_UNLOCK:
      case GEN_CMD:
    //case SWITCH_FUNC:
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_DBG: Data transfer not yet supported");
        #endif
        return;
      default:
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_ERR: Not an Addressed Data Transfer Command");
        #endif
        return;
    }
}

/**
 * @brief Sets up and handles polling data transfer for AppCommands
 */
void HardwareSDIO::transfer(SDAppCommand cmd) {
    switch (cmd) {
      case SD_STATUS:
      case SEND_SCR:
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_DBG: Data transfer in polling mode");
        #endif
        break;
      case SEND_NUM_WR_BLOCKS:
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_DBG: Data transfer not yet supported");
        #endif
        return;
      default:
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_ERR: Not an Addressed Data Transfer Command");
        #endif
        return;
    }

    if (this->CSR.READY_FOR_DATA == SDIO_CSR_READY) {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_DBG: Ready to receive data");
        #endif
    }

    while (1) {
        if (sdio_check_status(SDIO_STA_DTIMEOUT)) {
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("SDIO_ERR: Data timeout");
            #endif
            sdio_clear_interrupt(SDIO_ICR_DTIMEOUTC);
            return;
        }
        if (sdio_check_status(SDIO_STA_DCRCFAIL)) {
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("SDIO_ERR: Data CRC fail");
            #endif
            sdio_clear_interrupt(SDIO_ICR_DCRCFAILC);
            return;
        }
        if (sdio_check_status(SDIO_STA_STBITERR)) {
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("SDIO_ERR: Data start-bit");
            #endif
            return;
        }
        if (sdio_check_status(SDIO_STA_RXOVERR)) {
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("SDIO_ERR: Data FIFO overrun");
            #endif
            return;
        }
    }

    #if defined(SDIO_DEBUG_ON)
    DEBUG_DEVICE.println("SDIO_DBG: Transfer complete, no errors encounted");
    #endif

    uint32 buf[16]; //max 64-bytes or 512-bits in 32-bit words
    int rxed = 0;
    while (sdio_get_fifo_count() > 0) {
        buf[rxed++] = sdio_read_data();
    }
    #if defined(SDIO_DEBUG_ON)
    DEBUG_DEVICE.print("SDIO_DBG: Bytes received ");
    DEBUG_DEVICE.println(rxed*4, DEC);
    #endif

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
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_ERR: Polling data transfer not supported");
        #endif
        return;
    }
}

/**
 * Card register functions
 */

/**
 * @brief Card publishes a new Relative Card Address (RCA)
 */
void HardwareSDIO::newRCA(void) {
    this->command(SEND_RELATIVE_ADDR); //CMD3
    this->response(SEND_RELATIVE_ADDR);
    #if defined(SDIO_DEBUG_ON)
    DEBUG_DEVICE.print("SDIO_DBG: New RCA is 0x");
    DEBUG_DEVICE.println(this->RCA.RCA, HEX);
    DEBUG_DEVICE.print("SDIO_DBG: RESP1 0x");
    DEBUG_DEVICE.println(sdio_get_resp(1), HEX);
    #endif
}

/**
 * @brief Gets the Interface Condition Register (ICR)
 *        tells whether the operating voltage of the host is valid for the card
 * @note  Only allowed during identification mode
 */
void HardwareSDIO::getICR(void) {
    this->command(SEND_IF_COND, //CMD8
                //SDIO_SDXC_POWER_CONTROL |
                  SDIO_VOLTAGE_HOST_SUPPORT |
                  SDIO_CHECK_PATTERN);
    this->response(SEND_IF_COND);
    switch (this->IRQFlag) {
      case SDIO_FLAG_CMDREND:
        if (this->ICR.CHECK_PATTERN != SDIO_CHECK_PATTERN) {
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.print("SDIO_ERR: Unusuable Card, ");
            DEBUG_DEVICE.print("Check pattern 0x");
            DEBUG_DEVICE.println(this->ICR.CHECK_PATTERN, HEX);
            #endif
            return; // ASSERT(0);
        } else {
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("SDIO_DBG: Valid check pattern");
            #endif
        }
        if (this->ICR.VOLTAGE_ACCEPTED != SDIO_VOLTAGE_SUPPLIED) {
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.print("SDIO_ERR: Unusuable Card, ");
            DEBUG_DEVICE.print("Accepted voltage 0x");
            DEBUG_DEVICE.println(this->ICR.VOLTAGE_ACCEPTED, HEX);
            #endif
            return; // ASSERT(0);
        } else {
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("SDIO_DBG: Valid supplied voltage");
            #endif
        }
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_DBG: Interface condition check passed");
        #endif
        // version 2.0 card
        CSD.CSD_STRUCTURE = 1;
        break;
      default:
        return;
    }
}

/**
 * @brief Gets the Operating Conditions Register (OCR)
 * @note  Only allowed during identification mode
 */
void HardwareSDIO::getOCR(void) {
    this->command(SD_SEND_OP_COND, //ACMD41
                //SDIO_HOST_CAPACITY_SUPPORT | (OCR.VOLTAGE_WINDOW << 8),
                  SDIO_HOST_CAPACITY_SUPPORT | 
                  (SDIO_VALID_VOLTAGE_WINDOW << 8));
    this->response(SD_SEND_OP_COND);
}

/**
 * @brief Gets the Card IDentification number (CID)
 */
void HardwareSDIO::getCID(void) {
    this->command(SEND_CID, (uint32)RCA.RCA << 16); //CMD10
    this->response(SEND_CID);
}

/**
 * @brief Gets the Card Specific Data (CSD)
 */
void HardwareSDIO::getCSD(void) {
    this->command(SEND_CSD, (uint32)RCA.RCA << 16); //CMD9
    this->response(SEND_CSD);
    #if defined(SDIO_DEBUG_ON)
    DEBUG_DEVICE.print("SDIO_DBG: Card version ");
    switch (this->CSD.CSD_STRUCTURE) {
      case 0:
        DEBUG_DEVICE.println("1.x");
        break;
      case 1:
        DEBUG_DEVICE.println("2.0");
        break;
      default:
        DEBUG_DEVICE.println("Undefined");
        break;
    }
    DEBUG_DEVICE.print("SDIO_DBG: TAAC ");
    DEBUG_DEVICE.println(this->CSD.TAAC);
    DEBUG_DEVICE.print("SDIO_DBG: NSAC ");
    DEBUG_DEVICE.println(this->CSD.NSAC);
    DEBUG_DEVICE.print("SDIO_DBG: TRAN_SPEED ");
    DEBUG_DEVICE.println(this->CSD.TRAN_SPEED);
    DEBUG_DEVICE.print("SDIO_DBG: CCC ");
    DEBUG_DEVICE.println(this->CSD.CCC);
    DEBUG_DEVICE.print("SDIO_DBG: READ_BL_LEN ");
    DEBUG_DEVICE.println(this->CSD.READ_BL_LEN);
    DEBUG_DEVICE.print("SDIO_DBG: READ_BL_PARTIAL ");
    DEBUG_DEVICE.println(this->CSD.READ_BL_PARTIAL);
    DEBUG_DEVICE.print("SDIO_DBG: WRITE_BLK_MISALIGN ");
    DEBUG_DEVICE.println(this->CSD.WRITE_BLK_MISALIGN);
    DEBUG_DEVICE.print("SDIO_DBG: READ_BLK_MISALIGN ");
    DEBUG_DEVICE.println(this->CSD.READ_BLK_MISALIGN);
    DEBUG_DEVICE.print("SDIO_DBG: DSR_IMP ");
    DEBUG_DEVICE.println(this->CSD.DSR_IMP);
    DEBUG_DEVICE.print("SDIO_DBG: C_SIZE ");
    DEBUG_DEVICE.println(this->CSD.C_SIZE);
    #endif
}

/**
 * @brief Gets the Sd card Configuration Register (SCR)
 * @note Data packet format for Wide Width Data is most significant byte first
 */
void HardwareSDIO::getSCR(void) {
    this->blockLength(SDIO_BKSZ_8);
    sdio_set_data_timeout(SDIO_DTIMER_DATATIME);
    sdio_set_data_length(8); //64-bits or 8-bytes
    sdio_set_dcr((this->blockSize << SDIO_DCTRL_DBLOCKSIZE_BIT) |
                 SDIO_DCTRL_DTDIR);
    this->command(SEND_SCR); //ACMD51
    //this->response(SEND_SCR);
    this->transfer(SEND_SCR);

    #if defined(SDIO_DEBUG_ON)
    DEBUG_DEVICE.print("SDIO_DBG: SCR_STRUCTURE ");
    switch (this->SCR.SCR_STRUCTURE) {
      case 0:
        DEBUG_DEVICE.println("version 1.0");
        break;
      default:
        DEBUG_DEVICE.println("Reserved");
        break;
    }
    DEBUG_DEVICE.print("SDIO_DBG: SD_SPEC ");
    switch (this->SCR.SD_SPEC) {
      case 0:
        DEBUG_DEVICE.println("Version 1.0 and 1.01");
        break;
      case 1:
        DEBUG_DEVICE.println("Version 1.10");
        break;
      case 2:
        switch (this->SCR.SD_SPEC3) {
          case 0:
            DEBUG_DEVICE.println("Version 2.00");
            break;
          case 1:
            DEBUG_DEVICE.println("Version 3.0X");
            break;
          default:
            DEBUG_DEVICE.println("Version 2.00 or Version 3.0X");
            break;
        }
        break;
      default:
        DEBUG_DEVICE.println("Reserved");
        break;
    }
    DEBUG_DEVICE.print("SDIO_DBG: DATA_STAT_AFTER_ERASE ");
    DEBUG_DEVICE.println(this->SCR.DATA_STAT_AFTER_ERASE);
    DEBUG_DEVICE.print("SDIO_DBG: SD_SECURITY ");
    switch (this->SCR.SD_SECURITY) {
      case 0:
        DEBUG_DEVICE.println("No security");
        break;
      case 1:
        DEBUG_DEVICE.println("Not used");
        break;
      case 2:
        DEBUG_DEVICE.println("SDSC Card (Security Version 1.01)");
        break;
      case 3:
        DEBUG_DEVICE.println("SDHC Card (Security Version 2.00)");
        break;
      case 4:
        DEBUG_DEVICE.println("SDXC Card (Security Version 3.xx)");
        break;
      default:
        DEBUG_DEVICE.println("Reserved");
        break;
    }
    DEBUG_DEVICE.print("SDIO_DBG: EX_SECURITY ");
    if (this->SCR.EX_SECURITY) {
        DEBUG_DEVICE.println("Extended security is NOT supported");
    } else {
        DEBUG_DEVICE.println("Extended security is supported");
    }
    #endif
}

/**
 * @brief Gets Sd Status Register contents (SSR)
 * @param buf Buffer to store register data
 * @note Data packet format for Wide Width Data is most significant byte first
 */
void HardwareSDIO::getSSR(void) {
    sdio_set_data_timeout(SDIO_DTIMER_DATATIME);
    sdio_set_data_length(64); //512-bits or 64-bytes
    this->blockLength(SDIO_BKSZ_64);
    //this->select();
    sdio_set_dcr((this->blockSize << SDIO_DCTRL_DBLOCKSIZE_BIT) |
                 SDIO_DCTRL_DTDIR);
    this->command(SD_STATUS); //ACMD13
    //this->response(SD_STATUS);
    //this->check(0xFF9FC20);
    this->transfer(SD_STATUS);

    #if defined(SDIO_DEBUG_ON)
    DEBUG_DEVICE.print("SDIO_DBG: DAT_BUS_WIDTH ");
    switch (this->SSR.DAT_BUS_WIDTH) {
      case 0:
        DEBUG_DEVICE.println("1-bit width (default)");
        break;
      case 2:
        DEBUG_DEVICE.println("4-bit width (wide)");
        break;
      default: 
        DEBUG_DEVICE.println("Reserved or Error");
        break;
    }
    DEBUG_DEVICE.println("SDIO_DBG: SECURED_MODE ");
    if (this->SSR.SECURED_MODE) {
        DEBUG_DEVICE.print("In secured mode ");
    } else {
        DEBUG_DEVICE.print("NOT in secured mode ");
    }
    DEBUG_DEVICE.print("SDIO_DBG: SD_CARD_TYPE ");
    switch (this->SSR.SD_CARD_TYPE) {
      case 0:
        DEBUG_DEVICE.println("Regular SD R/W Card");
        break;
      case 1:
        DEBUG_DEVICE.println("SD ROM Card");
        break;
      case 2:
        DEBUG_DEVICE.println("OTP");
        break;
      default: 
        DEBUG_DEVICE.println("Unknown Card type");
        break;
    }
    DEBUG_DEVICE.print("SDIO_DBG: SIZE_OF_PROTECTED_AREA ");
    DEBUG_DEVICE.println(this->SSR.SIZE_OF_PROTECTED_AREA); //FIXME
    DEBUG_DEVICE.print("SDIO_DBG: SPEED_CLASS 0x");
    DEBUG_DEVICE.println(this->SSR.SPEED_CLASS, HEX);
    DEBUG_DEVICE.print("SDIO_DBG: PERFORMANCE_MOVE 0x");
    DEBUG_DEVICE.println(this->SSR.PERFORMANCE_MOVE, HEX);
    DEBUG_DEVICE.print("SDIO_DBG: AU_SIZE 0x");
    DEBUG_DEVICE.println(this->SSR.AU_SIZE, HEX);
    DEBUG_DEVICE.print("SDIO_DBG: ERASE_SIZE ");
    DEBUG_DEVICE.println(this->SSR.ERASE_SIZE);
    DEBUG_DEVICE.print("SDIO_DBG: ERASE_TIMEOUT ");
    DEBUG_DEVICE.println(this->SSR.ERASE_TIMEOUT);
    DEBUG_DEVICE.print("SDIO_DBG: ERASE_OFFSET ");
    DEBUG_DEVICE.println(this->SSR.ERASE_OFFSET);
    DEBUG_DEVICE.print("SDIO_DBG: UHS_SPEED_GRADE 0x");
    DEBUG_DEVICE.println(this->SSR.UHS_SPEED_GRADE, HEX);
    DEBUG_DEVICE.print("SDIO_DBG: UHS_AU_SIZE 0x");
    DEBUG_DEVICE.println(this->SSR.UHS_AU_SIZE, HEX);
    #endif
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
    //this->check(0xFF9FF00);
}

/**
 * @brief Selects the card with the memeber Relative Card Address
 */
void HardwareSDIO::select(void) {
    this->command(SELECT_DESELECT_CARD, this->RCA.RCA << 16); //CMD7
    //this->check(0xFF9FF00);
}

/**
 * @brief Deselects all cards, synonymous with select(0)
 */
void HardwareSDIO::deselect(void) {
    this->select(0);
}

/**
 * @brief Card sends its status register
 */
void HardwareSDIO::status(void) {
    this->command(SEND_STATUS, this->RCA.RCA << 16);
    this->response(SEND_STATUS);
}

/**
 * @brief Stops data stream transmission to/from card
 */
void HardwareSDIO::stop(void) {
    this->command(STOP_TRANSMISSION); //CMD12
    //this->check(0xC6F85E00);
}

/**
 * @brief Read
 * @param addr Card block address to read from
 * @param dst Local buffer on host, destination for data to be received
 * @count Number of blocks to be read, cannot be zero
 */
void HardwareSDIO::read(uint32 addr, uint32 *dst, uint32 count) {
    if (count == 0) {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_ERR: Block count of zero");
        #endif
        return;
    } else if (count == 1) {
        this->readBlock(addr, dst);
    }
    //FIXME: multiBlock read rountine to follow
}

/**
 * @brief Generic write command
 * @param addr Address on card to write to
 * @param scr Local buffer on host, source of data to be transmitted
 * @count Number of blocks to be written, cannot be zero
 */
void HardwareSDIO::write(uint32 addr, uint32 *src, uint32 count) {
    if (count == 0) {
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_ERR: Block count of zero");
        #endif
        return;
    } else if (count == 1) {
        this->writeBlock(addr, src);
    }
    //FIXME: multiBlock write rountine to follow
}

/**
 * @brief 
 * @param addr Card block address to read from
 * @param dst Local buffer on host, destination for data to be received
 * @note Data is send little-endian format
 */
void HardwareSDIO::readBlock(uint32 addr, uint32 *dst) {
    if (this->blockSize != SDIO_BKSZ_512) {
        this->blockLength(SDIO_BKSZ_512);
    } //FIXME: handle other block sizes for version 1 cards
    //FIXME: CCS must equal one for block unit addressing
    this->select();
    //check for busy signal on dat0 line?
    sdio_set_data_timeout(SDIO_DTIMER_DATATIME);
    sdio_set_data_length(0x1 << this->blockSize);
    sdio_cfg_dma_rx(dst, 0x1 << this->blockSize);
    this->command(READ_SINGLE_BLOCK, addr);
    //this->check(0xCFF9FE00);

    switch (this->IRQFlag) { 
      case SDIO_FLAG_CMDREND:
        this->response(READ_SINGLE_BLOCK);
        break;
      default:
        #if defined(SDIO_DEBUG_ON)
        DEBUG_DEVICE.println("SDIO_ERR: Unknown response in readBlock");
        #endif
        return;
    }
    while (1) {
        if (sdio_check_status(SDIO_STA_DTIMEOUT)) {
            sdio_clear_interrupt(SDIO_ICR_DTIMEOUTC);
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("SDIO_ERR: Data timeout");
            #endif
            return;
        } else if (sdio_check_status(SDIO_STA_DCRCFAIL)) {
            sdio_clear_interrupt(SDIO_ICR_DCRCFAILC);
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("SDIO_ERR: Data CRC fail");
            #endif
            return;
        } else if (sdio_check_status(SDIO_STA_STBITERR)) {
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("SDIO_ERR: Data start-bit");
            #endif
            return;
        } else if (sdio_check_status(SDIO_STA_RXOVERR)) {
            #if defined(SDIO_DEBUG_ON)
            DEBUG_DEVICE.println("SDIO_ERR: Data FIFO overrun");
            #endif
            return;
        } else if (sdio_check_status(SDIO_STA_DBCKEND)) {
            break;
        }
    }
    #if defined(SDIO_DEBUG_ON)
    DEBUG_DEVICE.print("SDIO_DBG: Transfer complete");
    #endif
    sdio_clear_interrupt(SDIO_ICR_DBCKENDC);
    sdio_dma_disable();
}

/**
 * @brief 
 * @param addr Card block address to write data to
  * @param src Local buffer source for data to be written
 * @note data is send little-endian format
 */
void HardwareSDIO::writeBlock(uint32 addr, uint32 *src) {
    /**
    1.  Do the card identification process 
    2.  Increase the SDIO_CK frequency 
    3.  Select the card by sending CMD7 
    4.  Configure the DMA2
    5.  Send CMD24 (WRITE_BLOCK)
    a)  Program the SDIO data length register (SDIO data timer register should 
        be already programmed before the card identification process)
    b)  Program the SDIO argument register with the address location of the 
        card where data is to be transferred
    c)  Program the SDIO command register: CmdIndex with 24 (WRITE_BLOCK); 
        WaitResp with ‘1’ (SDIO card host waits for a response); 
        CPSMEN with ‘1’ (SDIO card host enabled to send a command). 
        Other fields are at their reset value.
    d)  Wait for SDIO_STA[6] = CMDREND interrupt, 
        then program the SDIO data control register: DTEN with ‘1’ 
        (SDIO card host enabled to send data); 
        DTDIR with ‘0’ (from controller to card); 
        DTMODE with ‘0’ (block data transfer); 
        DMAEN with ‘1’ (DMA enabled); 
        DBLOCKSIZE with 0x9 (512 bytes). Other fields are don’t care.
    e)  Wait for SDIO_STA[10] = DBCKEND
    6.  Check that no channels are still enabled by polling the 
        DMA Enabled Channel Status register.
    */

    this->select();
    sdio_cfg_dma_tx(src, 0x1 << this->blockSize);
    this->command(WRITE_BLOCK, addr);
    sdio_dma_disable(); //FIXME
}