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
 * @file SDMC.h
 * @author Brian E Tovar <betovar@leaflabs.com>
 * @brief Wirish SD Memory Card implementation
 */

#include "sdio.h"
#include "libmaple_types.h"
#include "../wirish/Print.h"

#ifndef _SDMC_H_
#define _SDMC_H_

/**
 * SDIO Enumerations
 */

typedef enum SDIORespType {
    SDIO_RESP_NONE     = 0,
    SDIO_RESP_SHORT    = 1,
  //SDIO_RESP_NONE_2   = 2,
    SDIO_RESP_LONG     = 3,
    SDIO_RESP_TYPE1,
  //SDIO_RESP_TYPE1b,
    SDIO_RESP_TYPE2,
    SDIO_RESP_TYPE3,
  //SDIO_RESP_TYPE4,
  //SDIO_RESP_TYPE5,
    SDIO_RESP_TYPE6,
    SDIO_RESP_TYPE7
} SDIORespType;

typedef enum SDIOBusMode {
    SDIO_BUS_1BIT     = 0,
    SDIO_BUS_4BIT     = 1
  //SDIO_BUS_8BIT     = 2
} SDIOBusMode;

typedef enum SDIOClockFrequency {
  //SDIO_36_MHZ   = 0, // High Speed Mode not supported as of May 2012
    SDIO_24_MHZ   = 1,
    SDIO_18_MHZ   = 2,
    SDIO_12_MHZ   = 4,
    SDIO_6_MHZ    = 10,
    SDIO_3_MHZ    = 22,
    SDIO_2_MHZ    = 34,
    SDIO_1_MHZ    = 70,
    SDIO_500_KHZ  = 142,
    SDIO_400_KHZ  = 178,
    SDIO_300_KHZ  = 238,
    SDIO_CLK_INIT = SDIO_400_KHZ
} SDIOClockFrequency;

typedef enum SDIOBlockSize {
    SDIO_BKSZ_1       = 0,
    SDIO_BKSZ_2       = 1,
    SDIO_BKSZ_4       = 2,
    SDIO_BKSZ_8       = 3,
    SDIO_BKSZ_16      = 4,
    SDIO_BKSZ_32      = 5,
    SDIO_BKSZ_64      = 6,
    SDIO_BKSZ_128     = 7,
    SDIO_BKSZ_256     = 8,
    SDIO_BKSZ_512     = 9,
    SDIO_BKSZ_1024    = 10,
    SDIO_BKSZ_2048    = 11,
    SDIO_BKSZ_4096    = 12,
    SDIO_BKSZ_8192    = 13,
    SDIO_BKSZ_16384   = 14,
    SDIO_BKSZ_DEFAULT = 9
} SDIOBlockSize;

typedef enum SDIOInterruptFlag {
    SDIO_FLAG_CCRCFAIL  = 0,
    SDIO_FLAG_DCRCFAIL  = 1,
    SDIO_FLAG_CTIMEOUT  = 2,
    SDIO_FLAG_DTIMEOUT  = 3,
    SDIO_FLAG_TXUNDERR  = 4,
    SDIO_FLAG_RXOVERR   = 5,
    SDIO_FLAG_CMDREND   = 6,
    SDIO_FLAG_CMDSENT   = 7,
    SDIO_FLAG_DATAEND   = 8,
    SDIO_FLAG_STBITERR  = 9,
    SDIO_FLAG_DBCKEND   = 10,
    SDIO_FLAG_CMDACT    = 11,
    SDIO_FLAG_TXACT     = 12,
    SDIO_FLAG_RXACT     = 13,
    SDIO_FLAG_TXFIFOHE  = 14,
    SDIO_FLAG_RXFIFOHF  = 15,
    SDIO_FLAG_TXFIFOF   = 16,
    SDIO_FLAG_RXFIFOF   = 17,
    SDIO_FLAG_TXFIFOE   = 18,
    SDIO_FLAG_RXFIFOE   = 19,
    SDIO_FLAG_TXDAVL    = 20,
    SDIO_FLAG_RXDAVL    = 21,
    SDIO_FLAG_SDIOIT    = 22,
    SDIO_FLAG_CEATAEND  = 23,
    // added
    SDIO_FLAG_NONE      = 32,
    SDIO_FLAG_ERROR
} SDIOInterruptFlag;

/**
 * SD Command Enumerations
 */

typedef enum SDCommand {
// Basic Commands (class 0)
    /** CMD0 - Resets all cards to idle state */
    GO_IDLE_STATE           = 0,
    /** CMD1 - reserved MMC */
  //SEND_OP_COND            = 1,
    /** CMD2 - Asks any card to send the CID numbers on the CMD line */
    ALL_SEND_CID            = 2,
    /** CMD3 - Ask the card to publish a new relative address */
    SEND_RELATIVE_ADDR      = 3,
    /** CMD4 - Programs the DSR of all cards */
    SET_DSR                 = 4,
    /** CMD5 - reserved for SDIO cards */
  //IO_SEND_OP_COND         = 5,


    /** CMD7 - toggles a card between the stand-by and transfer
     * states or between the programming and disconnect states */
    SELECT_DESELECT_CARD    = 7,
    /** CMD8 - SD Memory Card interface condition */
    SEND_IF_COND            = 8,
    /** CMD9 - Addressed card sends its card-specific data (CSD)
     * on the CMD line */
    SEND_CSD                = 9,
    /** CMD10 - Addressed card sends its card identification (CID)
     * on the CMD line */
    SEND_CID                = 10,
    /** CMD11 - Switch to 1.8V bus signaling level */
  //VOLTAGE_SWITCH          = 11,
    /** CMD12 - Forces the card to stop transmission */
    STOP_TRANSMISSION       = 12,
    /** CMD13 - Addressed card sends its status register */
    SEND_STATUS             = 13,
    /** CMD14 - Reserved */
  //CMD14                   = 14,
    /** CMD15 - Sends an addressed card into the Inactive State */
    GO_INACTIVE_STATE       = 15,

// Block-Oriented Read Commands (class 2)
    /** CMD16 -  */
    SET_BLOCKLEN            = 16,
    /** CMD17 -  */
    READ_SINGLE_BLOCK       = 17,
    /** CMD18 -  */
    READ_MULTIPLE_BLOCK     = 18,
    /** CMD19 - Reserved */
  //SEND_TUNING_BLOCK       = 19,
    /** CMD20 - Reserved */
  //SPEED_CLASS_CONTROL     = 20,
    /** CMD21 - Reserved */
  //CMD21                   = 21,
    /** CMD22 - Reserved */
  //CMD22                   = 22,
    /** CMD23 -  */
    SET_BLOCK_COUNT         = 23,

// Block-Oriented Write Commands (class 4)
    /** CMD16 - Repeated from class 2 */
  //SET_BLOCKLEN            = 16,
    /** CMD20 - Repeated from class 2 */
  //SPEED_CLASS_CONTROL     = 20,
    /** CMD23 - Repeated from class 2 */
  //SET_BLOCK_COUNT         = 23,
    /** CMD24 -  */
    WRITE_BLOCK             = 24,
    /** CMD25 -  */
    WRITE_MULTIPLE_BLOCK    = 25,
    /** CMD26 - Reserved for manufacturer */
  //PROGRAM_CID             = 26,
    /** CMD27 -  */
    PROGRAM_CSD             = 27,

// Block Oriented Write Protection Commands (class 6)
    /** CMD28 - Set Write Protect */
    SET_WRITE_PROT          = 28,
    /** CMD29 - Clear Write Protect */
    CLR_WRITE_PROT          = 29,
    /** CMD30 - Send Write Protect */
    SEND_WRITE_PROT         = 30,
    /** CMD31 - Reserved */
  //CMD31                   = 31,

// Erase Commands (class 5)
    /** CMD32 -  */
    ERASE_WR_BLK_START_ADDR = 32,
    /** CMD33 -  */
    ERASE_WR_BLK_END_ADDR   = 33,
    /** CMD38 -  */
    ERASE                   = 38,
    /** CMD39 -  */
  //CMD39                   = 39,
    /** CMD41 - Reserved */
  //CMD41                   = 41,

// Lock Card (class 7)
    /** CMD16 - Repeated from class 2 */
  //SET_BLOCKLEN            = 16,
    /** CMD40 -  */
  //CMD40                   = 40,
    /** CMD42 -  */
    LOCK_UNLOCK             = 42,
    /** CMD43-49, 51 - Reserved */

// Application Specific Commands (class 8)
    /** CMD55 -  */
    APP_CMD                 = 55,
    /** CMD56 -  */
    GEN_CMD                 = 56
    /** CMD58 - Reserved */
  //READ_OCR                = 58,
    /** CMD59 - Reserved */
  //CRC_ON_OFF              = 59,
    /** CMD60-63 - Reserved for manufacturer */

// I/O Mode Commands (class 9)
    /** CMD52-54 - Commands for SDIO */
  //IO_RW_DIRECT            = 52,
  //IO_RW_EXTENDED          = 53,
  //CMD54                   = 54,

// Switch Function Commands (class 10)
    /** CMD6 -  */
  //SWITCH_FUNC             = 6,
    /** CMD34 -  */
  //CMD34                   = 34,
    /** CMD35 -  */
  //CMD35                   = 35,
    /** CMD36 -  */
  //CMD36                   = 36,
    /** CMD37 -  */
  //CMD37                   = 37,
    /** CMD50 -  */
  //CMD50                   = 50,
    /** CMD57 -  */
  //CMD57                   = 57,
} SDCommand;

typedef enum SDAppCommand {
// Application Specific Commands used/reserved by SD Memory Card (class n/a)
    /** ACMD1-5 - Reserved */
    /** ACMD6 -  */
    SET_BUS_WIDTH           = 6,
    /** ACMD7-12 - Reserved */
    /** ACMD13 -  */
    SD_STATUS               = 13,
    /** ACMD14-21 - Reserved */
    /** ACMD22 -  */
    SEND_NUM_WR_BLOCKS      = 22,
    /** ACMD23 -  */
    SET_WR_BLK_ERASE_COUNT  = 23,
    /** ACMD24-40 - Reserved */
    /** ACMD41 -  */
    SD_SEND_OP_COND         = 41,
    /** ACMD42 -  */
    SET_CLR_CARD_DETECT     = 42,
    /** ACMD51 -  */
    SEND_SCR                = 51
    /** ACMD52-59 - Reserved */
} SDAppCommand;

/**
 * SD Structure Specific Enumerations
 */

typedef enum CsdCardVersion {
    CSD_VER_UNDEF = 0,
    CSD_VER_1     = 1,
    CSD_VER_2     = 2
} CsdCardVersion;

typedef enum CsdCardCapacity {
    CSD_CAP_UNDEF = 0,
    CSD_CAP_SDSC  = 1,
    CSD_CAP_SDHC  = 2,
    CSD_CAP_SDXC  = 3
} CsdCardCapacity;

typedef enum SDIOStatusResponseTag {
    //CardStatusResponse Tags
    SDIO_CSR_NO_ERROR        = 0,
    SDIO_CSR_ERROR           = 1,
    SDIO_CSR_NOT_PROTECTED   = 0,
    SDIO_CSR_PROTECTED       = 1,
    SDIO_CSR_CARD_UNLOCKED   = 0,
    SDIO_CSR_CARD_LOCKED     = 1,
    SDIO_CSR_SUCCESS         = 0,
    SDIO_CSR_FAILURE         = 1,
    SDIO_CSR_ECC_ENABLED     = 0,
    SDIO_CSR_ECC_DISABLED    = 1,
    SDIO_CSR_CLEARED         = 0,
    SDIO_CSR_SET             = 1,
    SDIO_CSR_IDLE            = 0,
    SDIO_CSR_NOT_READY       = 0,
    SDIO_CSR_READY           = 1,
    SDIO_CSR_IDENT           = 2,
    SDIO_CSR_STBY            = 3,
    SDIO_CSR_TRAN            = 4,
    SDIO_CSR_DATA            = 5,
    SDIO_CSR_RCV             = 6,
    SDIO_CSR_PRG             = 7,
    SDIO_CSR_DIS             = 8,
    SDIO_CSR_IO_MODE         = 15,
    SDIO_CSR_DISABLED        = 0,
    SDIO_CSR_ENABLED         = 1,
    //SdStatusResponse Tags
    SDIO_SSR_1BIT_WIDTH      = 0,
    SDIO_SSR_4BIT_WIDTH      = 2,
    SDIO_SSR_NOT_SECURED     = 0,
    SDIO_SSR_SECURED         = 1,
    SDIO_SSR_REG_CARD        = 0,
    SDIO_SSR_ROM_CARD        = 1,
    SDIO_SSR_OTP_CARD        = 2,
    SDIO_SSR_SPEED_CLASS_0   = 0,
    SDIO_SSR_SPEED_CLASS_2   = 1,
    SDIO_SSR_SPEED_CLASS_4   = 2,
    SDIO_SSR_SPEED_CLASS_6   = 3,
    SDIO_SSR_SPEED_CLASS_10  = 4,
    SDIO_SSR_AU_SIZE_NOT_DEF = 0,
    SDIO_SSR_AU_SIZE_16KB    = 1,
    SDIO_SSR_AU_SIZE_32KB    = 2,
    SDIO_SSR_AU_SIZE_64KB    = 3,
    SDIO_SSR_AU_SIZE_128KB   = 4,
    SDIO_SSR_AU_SIZE_256KB   = 5,
    SDIO_SSR_AU_SIZE_512KB   = 6,
    SDIO_SSR_AU_SIZE_1MB     = 7,
    SDIO_SSR_AU_SIZE_2MB     = 8,
    SDIO_SSR_AU_SIZE_4MB     = 9,
    SDIO_SSR_AU_SIZE_8MB     = 10,
    SDIO_SSR_AU_SIZE_12MB    = 11,
    SDIO_SSR_AU_SIZE_16MB    = 12,
    SDIO_SSR_AU_SIZE_24MB    = 13,
    SDIO_SSR_AU_SIZE_32MB    = 14,
    SDIO_SSR_AU_SIZE_64MB    = 15,
} SDIOStatusResponseTag;

/**
 * SD Register Structures
 */

typedef struct OperationConditionsRegister {//little-endian
    /** Reserved for low voltage range */
    unsigned Reserved3              :8;
    /** VDD Voltage Window: 2.7v - 3.6v */
    unsigned VOLTAGE_WINDOW         :16;
    /** Switch to 1.8v Accepted:
     *  Only UHS-I card supports this bit */
    unsigned S18A                   :1;
    unsigned Reserved2              :4;
    unsigned Reserved1              :1;
    /** Card Capacity Status: This bit is valid only when
     *  the card power up status bit is set. 
     *  SDHC and SDXC use the 32-bit argument of memory access commands as
     *  block address format. Block length is fixed to 512 bytes regardless
     *  CMD16, SDSC uses the 32-bit argument of memory access commands as byte
     *  address format. Block length is determined by CMD16
     */
    unsigned CCS                    :1;
    /** Card power up status bit: This bit is set to LOW if
     *  the card has not finished the power up routine. */
    unsigned BUSY                   :1;
}__attribute__((packed)) ocr;

typedef struct product_revision {
    unsigned N                      :4;
    unsigned M                      :4;
}__attribute__((packed)) prod_revn;

typedef struct manufacturing_date {
    unsigned Rerserved1             :4;
    unsigned YEAR                   :8;
    unsigned MONTH                  :4;
}__attribute__((packed)) manu_date;

typedef struct CardIdentificationRegister {
  /** An 8-bit binary number that identifies the card manufacturer */
  uint8 MID; // Manufacturer ID
  /** A 2-character ASCII string that identifies the card OEM */
  char OID[2]; // OEM/Application ID
  /** The product name is a string, 5-character ASCII string */
  char PNM[5]; // Product Name
  /** The product revision is composed of two Binary Coded Decimal (BCD)
   * digits, four bits each, repre- senting an "n.m" revision number.
   * The "n" is the most significant nibble and "m" is the least
   * significant nibble */
  prod_revn PRN; // Product Revision Number
  /** The Serial Number is 32 bits of a binary number */
  uint32 PSN;  // Product Serial Number
  /** The manufacturing date is composed of two hexadecimal digits, one
   * is 8 bits representing the year(y) and the other is 4 bits representing
   * the month (m). The "m" field [11:8] is the month code. 1 = January.
   * The "y" field [19:12] is the year code. 0 = 2000. */
  manu_date MDT; // Manufacturing Date, most significant 4 bits are reserved
  /** CRC7 checksum (7 bits), the  zeroth bit is always 1 */
  unsigned CRC                      :7; // CRC7 Checksum
  unsigned Always1                  :1;
}__attribute__((packed)) cid;

//RelativeCardAddress
typedef struct RelativeCardAddress { //litte-endian
    uint8 Reserved2;
    uint8 Reserved1;
    uint16 RCA;
}__attribute__((packed)) rca;

// DriverStageRegister
typedef uint16 dsr; // Optional

typedef struct CardSpecificDataV1 {
    unsigned Always1                :1;
    unsigned CRC                    :7;
    unsigned Reserved5              :2;
    unsigned FILE_FORMAT            :2;
    unsigned TMP_WRITE_PROTECT      :1;
    unsigned PERM_WRITE_PROTECT     :1;
    unsigned COPY                   :1;
    unsigned FILE_FORMAT_GRP        :1;
    unsigned Reserved4              :5;
    unsigned WRITE_BL_PARTIAL       :1;
    unsigned WRITE_BL_LEN           :4;
    unsigned R2W_FACTOR             :3;
    unsigned Reserved3              :2;
    unsigned WP_GRP_ENABLE          :1;
    unsigned WP_GRP_SIZE            :7;
    unsigned SECTOR_SIZE            :7;
    unsigned ERASE_BLK_EN           :1;
    unsigned C_SIZE_MULT            :3;
    unsigned VDD_W_CURR_MAX         :3;
    unsigned VDD_W_CURR_MIN         :3;
    unsigned VDD_R_CURR_MAX         :3;
    unsigned VDD_R_CURR_MIN         :3;
    unsigned C_SIZE                 :12;
    unsigned Reserved2              :2;
    unsigned DSR_IMP                :1;
    unsigned READ_BLK_MISALIGN      :1;
    unsigned WRITE_BLK_MISALIGN     :1;
    unsigned READ_BL_PARTIAL        :1;
    unsigned READ_BL_LEN            :4;
    unsigned CCC                    :12;
    uint8 TRAN_SPEED;
    uint8 NSAC;
    uint8 TAAC;
    unsigned Reserved1              :6;
    unsigned CSD_STRUCTURE          :2;
}__attribute__((packed)) csdV1;

typedef struct CardSpecificDataV2 {
    unsigned Always1                :1;
    unsigned CRC                    :7;
    unsigned Reserved6              :2;
    unsigned FILE_FORMAT            :2;
    unsigned TMP_WRITE_PROTECT      :1;
    unsigned PERM_WRITE_PROTECT     :1;
    unsigned COPY                   :1;
    unsigned FILE_FORMAT_GRP        :1;
    unsigned Reserved5              :5;
    unsigned WRITE_BL_PARTIAL       :1;
    unsigned WRITE_BL_LEN           :4;
    unsigned R2W_FACTOR             :3;
    unsigned Reserved4              :2;
    unsigned WP_GRP_ENABLE          :1;
    unsigned WP_GRP_SIZE            :7;
    unsigned SECTOR_SIZE            :7;
    unsigned ERASE_BLK_EN           :1;
    unsigned Reserved3              :1;
    unsigned C_SIZE                 :22;
    unsigned Reserved2              :6;
    unsigned DSR_IMP                :1;
    unsigned READ_BLK_MISALIGN      :1;
    unsigned WRITE_BLK_MISALIGN     :1;
    unsigned READ_BL_PARTIAL        :1;
    unsigned READ_BL_LEN            :4;
    unsigned CCC                    :12;
    uint8 TRAN_SPEED;
    uint8 NSAC;
    uint8 TAAC;
    unsigned Reserved1              :6;
    unsigned CSD_STRUCTURE          :2;
}__attribute__((packed)) csdV2;

typedef struct CardSpecificData {
    union {
        csdV1 V1;
        csdV2 V2;
    };
    CsdCardVersion version;
    CsdCardCapacity capacity;
}__attribute__((packed)) csd;

typedef struct SdConfigurationRegister {
    /** value 0 is for physical layer spec 1.01-3.01 */
    unsigned SCR_STRUCTURE          :4;
    /** SD Memory Card - Spec. Version */
    unsigned SD_SPEC                :4;
    /** The data status is card vendor dependent */
    unsigned DATA_STAT_AFTER_ERASE  :1;
    /** CPRM Security Specification Version */
    unsigned SD_SECURITY            :3;
    /** DAT bus widths that are supported by the card */
    unsigned SD_BUS_WIDTHS          :4;
    /** Spec. Version 3.00 or higher */
    unsigned SD_SPEC3               :1;
    /** Extended Security support */
    unsigned EX_SECURITY            :4;
    unsigned Reserved1              :9;
    /** new command support for newer cards */
    unsigned CMD_SUPPORT            :2;
    /** Reserved for manufacturer */
    uint32 Reserved2;
}__attribute__((packed)) scr;

/**
 * Response Structures
 */

typedef struct CardStatusResponse { //litte-endian
    unsigned Reserved6              :2;
    unsigned Reserved5              :1;
    unsigned AKE_SEQ_ERROR          :1;
    unsigned Reserved4              :1;
    unsigned APP_CMD                :1;
    unsigned Reserved3              :2;

    unsigned READY_FOR_DATA         :1;
    unsigned CURRENT_STATE          :4;
    unsigned ERASE_RESET            :1;
    unsigned CARD_ECC_DISABLED      :1;
    unsigned WP_ERASE_SKIP          :1;

    unsigned CSD_OVERWRITE          :1;
    unsigned Reserved2              :1;
    unsigned Reserved1              :1;
    unsigned ERROR                  :1;
    unsigned CC_ERROR               :1;
    unsigned CARD_ECC_FAILED        :1;
    unsigned ILLEGAL_COMMAND        :1;
    unsigned COM_CRC_ERROR          :1;

    unsigned LOCK_UNLOCK_FAILED     :1;
    unsigned CARD_IS_LOCKED         :1;
    unsigned WP_VIOLATION           :1;
    unsigned ERASE_PARAM            :1;
    unsigned ERASE_SEQ_ERROR        :1;
    unsigned BLOCK_LEN_ERROR        :1;
    unsigned ADDRESS_ERROR          :1;
    unsigned OUT_OF_RANGE           :1;
}__attribute__((packed)) csr;

typedef struct SdStatusResponse {
    unsigned DAT_BUS_WIDTH          :2;
    unsigned SECURED_MODE           :1;
    unsigned Reserved1              :7;
    unsigned Reserved2              :6;
    uint16 SD_CARD_TYPE;
    uint32 SIZE_OF_PROTECTED_AREA;
    uint8 SPEED_CLASS;
    uint8 PERFORMANCE_MOVE;
    unsigned AU_SIZE                :4;
    unsigned Reserved3              :4;
    uint16 ERASE_SIZE;
    unsigned ERASE_TIMEOUT          :6;
    unsigned ERASE_OFFSET           :2;
    unsigned UHS_SPEED_GRADE        :4;
    unsigned UHS_AU_SIZE            :4;
    uint8 Reserved4[10];
    uint8 Reserved5[39];
}__attribute__((packed)) ssr;

typedef struct InterfaceConditionResponse { //litte-endian
    unsigned CHECK_PATTERN          :8;
    unsigned VOLTAGE_ACCEPTED       :4;
    unsigned Reserved1              :20;
}__attribute__((packed)) icr;

/** SDIO Card Structures, TBD

//CardCommonControlRegister
typedef struct CardCommonControlRegister {} cccr;

//FunctionBasicRegister
typedef struct FunctionBasicRegister {} fbr;

//CardInformationStructure
typedef struct CardInformationStructure {} cis;

/CodeStorageArea
typedef struct CodeStorageArea {} csa;
*/

class SecureDigitalMemoryCard {
  public:
    ocr OCR;
    scr SCR;
    ssr SSR;
    cid CID;
    csd CSD;
    rca RCA;
    dsr DSR; // Default is 0x0404
    csr CSR;
    
    SDIOInterruptFlag ruptFlag;
    SDIOInterruptFlag ruptFlag55;

    SecureDigitalMemoryCard(void);
    //---------------- startup functions ------------------
    void begin(void);
    void end(void);
    //---------------- convenience functions --------------
    void idle(void);
    void clockFreq(SDIOClockFrequency);
    void busMode(SDIOBusMode);
    void blockSize(SDIOBlockSize);
    //---------------- command functions ------------------
    void cmd(SDCommand);
    void cmd(SDCommand, uint32);
    void cmd(SDCommand, uint32, SDIORespType, uint32*);
    void cmd(SDCommand, uint32, SDIORespType, uint32*, uint32);
    void cmd(SDAppCommand);
    void cmd(SDAppCommand, uint32);
    void cmd(SDAppCommand, uint32, SDIORespType, uint32*);
    void cmd(SDAppCommand, uint32, SDIORespType, uint32*, uint32);
    void response(SDCommand);
    //---------------- general data functions -------------
    void stop(void);
    void read(uint32, uint32*, uint32);
    void write(uint32, const uint32*, uint32);
    //---------------- card register access functions -----
    void getCID(void);
    void getCSD(void);
    void getSCR(void);
    void getSSR(uint32*);
    void setDSR(void);

  private:
    sdio_dev *sdio_d;
    //---------------- begin routines ---------------------
    void initialization(void);
    void identification(void);
    void getOCR(void);
    void newRCA(void);
    //---------------- card select functions --------------
    void select(uint16);
    void deselect(void);
    //---------------- basic data functions ---------------
    void readBlock(uint32, uint32*);
    void writeBlock(uint32, const uint32*);
    
    /** other functions to be developed
    void protect(void); // write protect
    void passwordSet(void);
    void passwordReset(void);
    void cardLock(void);
    void cardUnlock(void);
    void erase(void);
    */
};

#endif