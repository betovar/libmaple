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
 * @file commands.h
 * @author Brian E Tovar <betovar@leaflabs.com>
 * @brief SD/MMC command listing
 */

/*  //FIXME temporary, replace these with more general routines
static const uint32 SDIO_SWITCH_1V8_REQUEST     = 0x1 << 24; //Not allowed
static const uint32 SDIO_SDXC_POWER_CONTROL     = 0x1 << 28; 
static const uint32 SDIO_FAST_BOOT              = 0x1 << 29; //Reserved      */
static const uint32 SDIO_HOST_CAPACITY_SUPPORT  = 0x1 << 30;
static const uint32 SDIO_CHECK_PATTERN          = 0xAA;
static const uint32 SDIO_VOLTAGE_SUPPLIED       = 0x1;
static const uint32 SDIO_VOLTAGE_HOST_SUPPORT   = SDIO_VOLTAGE_SUPPLIED << 8;
static const uint32 SDIO_VALID_VOLTAGE_WINDOW   = 0x3000; //3.2-3.4 volts

typedef enum SDCommand {
// Basic Commands (class 0)
    /* CMD0 - Resets all cards to idle state */
    GO_IDLE_STATE           = 0,
    /* CMD1 - Valid for the Thin (1.4mm) Standard Size SD Memory Card only if
     * used after re-initializing a card (not after power on reset) */
    SEND_OP_COND            = 1,
    /* CMD2 - Asks any card to send the CID numbers on the CMD line */
    ALL_SEND_CID            = 2,
    /* CMD3 - Ask the card to publish a new relative address */
    SEND_RELATIVE_ADDR      = 3,
    /* CMD4 - Programs the DSR of all cards */
    SET_DSR                 = 4,
    /* CMD5 - reserved for SDIO cards */
  //IO_SEND_OP_COND         = 5,
    /* CMD7 - toggles a card between the stand-by and transfer
     * states or between the programming and disconnect states */
    SELECT_DESELECT_CARD    = 7,
    /* CMD8 - SD Memory Card interface condition */
    SEND_IF_COND            = 8,
    /* CMD9 - Addressed card sends its card-specific data (CSD)
     * on the CMD line */
    SEND_CSD                = 9,
    /* CMD10 - Addressed card sends its card identification (CID)
     * on the CMD line */
    SEND_CID                = 10,
    /* CMD11 - Switch to 1.8V bus signaling level */
  //VOLTAGE_SWITCH          = 11,
    /* CMD12 - Forces the card to stop transmission */
    STOP_TRANSMISSION       = 12,
    /* CMD13 - Addressed card sends its status register */
    SEND_STATUS             = 13,
    /* CMD14 - Reserved */
  //CMD14                   = 14,
    /* CMD15 - Sends an addressed card into the Inactive State */
    GO_INACTIVE_STATE       = 15,

// Block-Oriented Read Commands (class 2)
    /* CMD16 -  */
    SET_BLOCKLEN            = 16,
    /* CMD17 -  */
    READ_SINGLE_BLOCK       = 17,
    /* CMD18 -  */
    READ_MULTIPLE_BLOCK     = 18,
    /* CMD19 - Reserved */
  //SEND_TUNING_BLOCK       = 19,
    /* CMD20 - Reserved */
  //SPEED_CLASS_CONTROL     = 20,
    /* CMD21 - Reserved */
  //CMD21                   = 21,
    /* CMD22 - Reserved */
  //CMD22                   = 22,
    /* CMD23 -  */
    SET_BLOCK_COUNT         = 23,

// Block-Oriented Write Commands (class 4)
    /* CMD24 -  */
    WRITE_BLOCK             = 24,
    /* CMD25 -  */
    WRITE_MULTIPLE_BLOCK    = 25,
    /* CMD26 - Reserved for manufacturer */
  //PROGRAM_CID             = 26,
    /* CMD27 -  */
    PROGRAM_CSD             = 27,

// Block Oriented Write Protection Commands (class 6)
    /* CMD28 - Set Write Protect */
    SET_WRITE_PROT          = 28,
    /* CMD29 - Clear Write Protect */
    CLR_WRITE_PROT          = 29,
    /* CMD30 - Send Write Protect */
    SEND_WRITE_PROT         = 30,
    /* CMD31 - Reserved */
  //CMD31                   = 31,

// Erase Commands (class 5)
    /* CMD32 -  */
    ERASE_WR_BLK_START_ADDR = 32,
    /* CMD33 -  */
    ERASE_WR_BLK_END_ADDR   = 33,
    /* CMD35 -  */
  //ERASE_GROUP_START       = 35,
    /* CMD36 -  */
  //ERASE_GROUP_END         = 36,
    /* CMD38 -  */
    ERASE                   = 38,

// Lock Card (class 7)
    /* CMD42 -  */
    LOCK_UNLOCK             = 42,
    /* CMD43-49, 51 - Reserved */

// Application Specific Commands (class 8)
    /* CMD55 -  */
    APP_CMD                 = 55,
    /* CMD56 -  */
    GEN_CMD                 = 56
    /* CMD58 - Reserved */
  //READ_OCR                = 58,
    /* CMD59 - Reserved */
  //CRC_ON_OFF              = 59,
    /* CMD60-63 - Reserved for manufacturer */

// I/O Mode Commands (class 9)
    /* CMD39 -  */
  //FAST_IO                 = 39,
    /* CMD40 -  */
  //GO_IRQ_STATE            = 40,
    /* CMD41 - Reserved */
  //CMD41                   = 41,
    /* CMD52-54 - Commands for SDIO */
  //IO_RW_DIRECT            = 52,
  //IO_RW_EXTENDED          = 53,
  //CMD54                   = 54,

// Switch Function Commands (class 10)
    /* CMD6 - Checks switchable function and switches card function */
  //SWITCH_FUNC             = 6,
    /* CMD34 -  */
  //CMD34                   = 34,
    /* CMD37 - Reserved of MMC */
  //CMD37                   = 37,
    /* CMD50 -  */
  //CMD50                   = 50,
    /* CMD57 -  */
  //CMD57                   = 57,
} SDCommand;

typedef enum SDAppCommand {
// Application Specific Commands used/reserved by SD Memory Card (class n/a)
    /* ACMD1-5 - Reserved */
    /* ACMD6 -  */
    SET_BUS_WIDTH           = 6,
    /* ACMD7-12 - Reserved */
    /* ACMD13 -  */
    SD_STATUS               = 13,
    /* ACMD14-21 - Reserved */
    /* ACMD22 -  */
    SEND_NUM_WR_BLOCKS      = 22,
    /* ACMD23 -  */
    SET_WR_BLK_ERASE_COUNT  = 23,
    /* ACMD24-40 - Reserved */
    /* ACMD41 -  */
    SD_SEND_OP_COND         = 41,
    /* ACMD42 -  */
    SET_CLR_CARD_DETECT     = 42,
    /* ACMD51 -  */
    SEND_SCR                = 51
    /* ACMD52-59 - Reserved */
} SDAppCommand;