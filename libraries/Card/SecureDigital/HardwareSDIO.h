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
 * @file HardwareSDIO.h
 * @author Brian E Tovar <betovar@leaflabs.com>
 * @brief Wirish SD Memory Card implementation
 * @note These devices share DMA Channel4: TIM5_CH2 SDIO TIM7_UP/DAC_Channel2
 */

#include <libmaple/libmaple_types.h>
#include <Card/SecureDigital/commands.h>
#include <libmaple/sdio.h>
 

#ifndef _HARDWARESDIO_H_
#define _HARDWARESDIO_H_

/*
 * SDIO Enumerations
 */

typedef enum SDIOBusMode {
    SDIO_BUS_1BIT     = 0,
    SDIO_BUS_4BIT     = 1
  //SDIO_BUS_8BIT     = 2 // used for UHS cards
} SDIOBusMode;

typedef enum SDIOClockFrequency {
  //SDIO_36_MHZ   = 0, // High Speed Mode not yet supported
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
    SDIO_CLK_INIT = SDIO_300_KHZ //254 //281,250 Hz
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

/*
 * SD Structure Specific Enumerations
 */

typedef enum SdCardCapacity { //FIXME: don't see the need to use this
    SD_CAP_UNDEF = 0,
    SD_CAP_SDSC  = 1,
    SD_CAP_SDHC  = 2,
    SD_CAP_SDXC  = 3
} SdCardCapacity;

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

/*
 * SD Register Structures
 */

typedef struct InterfaceConditionResponse {
  //unsigned Reserved               :20;
    unsigned VOLTAGE_ACCEPTED       :4;
    unsigned CHECK_PATTERN          :8;
} icr;

typedef struct OperationConditionsRegister { //MSBit first
    /* Card power up status bit: This bit is set to LOW if
     * the card has not finished the power up routine. */
    unsigned BUSY                   :1;
    /* Card Capacity Status: This bit is valid only when
     * the card power up status bit is set. 
     * SDHC and SDXC use the 32-bit argument of memory access commands as
     * block address format. Block length is fixed to 512 bytes regardless
     * CMD16, SDSC uses the 32-bit argument of memory access commands as byte
     * address format. Block length is determined by CMD16
     */
    unsigned CCS                    :1;
    /* Switch to 1.8v Accepted:
     * Only UHS-I card supports this bit */
    unsigned S18A                   :1;
    unsigned VOLTAGE_WINDOW         :16; // 2.7v - 3.6v
  //unsigned Reserved               :8; // Reserved for low voltage range
} ocr;

typedef struct product_revision {
    unsigned N                      :4;
    unsigned M                      :4;
}__attribute__((packed)) prod_revn;

typedef struct manufacturing_date {
    unsigned YEAR                   :8;
    unsigned MONTH                  :4;
}__attribute__((packed)) manu_date;

typedef struct CardIdentificationNumber { //MSBit first
    /* An 8-bit binary number that identifies the card manufacturer */
    uint8 MID; // Manufacturer ID
    /* A 2-character ASCII string that identifies the card OEM */
    char OID[3]; // OEM/Application ID, modified to be a NULL terminated string
    /** The product name is a string, 5-character ASCII string */
    char PNM[6]; // Product Name, modified to be a NULL terminated string
    /* The Serial Number is 32 bits of a binary number */
    uint32 PSN;  // Product Serial Number
    /* The product revision is composed of two Binary Coded Decimal (BCD)
     * digits, four bits each, repre- senting an "n.m" revision number.
     * The "n" is the most significant nibble and "m" is the least
     * significant nibble */
    prod_revn PRV; // Product Revision Number
  //unsigned Reserved1                :4;
    /* The manufacturing date is composed of two hexadecimal digits, one
     * is 8 bits representing the year(y) and the other is 4 bits representing
     * the month (m). The "m" field [11:8] is the month code. 1 = January.
     * The "y" field [19:12] is the year code. 0 = 2000. */
    manu_date MDT; // Manufacturing Date, most significant 4 bits are reserved
    /** CRC7 checksum (7 bits) */
    unsigned CRC                      :7;
    /* ST specific: The SDIO_RESP4 register LSBit is always 0b */
  //unsigned Always1                  :1;
} cid;

typedef struct RelativeCardAddress { //MSBit first
    uint16 RCA;
    unsigned COM_CRC_ERROR          :1;
    unsigned ILLEGAL_COMMAND        :1;
    unsigned ERROR                  :1;
    unsigned CURRENT_STATE          :4;
    unsigned READY_FOR_DATA         :1;
  //unsigned Reserved3              :2; //copied from CSR
    unsigned APP_CMD                :1;
  //unsigned Reserved4              :1;
    unsigned AKE_SEQ_ERROR          :1;
  //unsigned Reserved5              :1;
  //unsigned Reserved6              :2;
} rca;

typedef uint16 dsr; // DriverStageRegister is optional

typedef struct CardSpecificData {
    SdCardCapacity capacity;
    unsigned CSD_STRUCTURE          :2;
  //unsigned Reserved1              :6;
    uint8 TAAC;
    uint8 NSAC;
    uint8 TRAN_SPEED;
    unsigned CCC                    :12;
    unsigned READ_BL_LEN            :4;
    unsigned READ_BL_PARTIAL        :1;
    unsigned WRITE_BLK_MISALIGN     :1;
    unsigned READ_BLK_MISALIGN      :1;
    unsigned DSR_IMP                :1;
    unsigned C_SIZE                 :22; // hack: ver1 cards C_SIZE is 12-bits
    unsigned VDD_R_CURR_MIN         :3;  // hack: available on ver1 cards only
    unsigned VDD_R_CURR_MAX         :3;  // hack: available on ver1 cards only
    unsigned VDD_W_CURR_MIN         :3;  // hack: available on ver1 cards only
    unsigned VDD_W_CURR_MAX         :3;  // hack: available on ver1 cards only
    unsigned C_SIZE_MULT            :3;  // hack: available on ver1 cards only
    unsigned ERASE_BLK_EN           :1;
    unsigned SECTOR_SIZE            :7;
    unsigned WP_GRP_SIZE            :7;
    unsigned WP_GRP_ENABLE          :1;
    unsigned R2W_FACTOR             :3;
    unsigned WRITE_BL_LEN           :4;
    unsigned WRITE_BL_PARTIAL       :1;
    unsigned FILE_FORMAT_GRP        :1;
    unsigned COPY                   :1;
    unsigned PERM_WRITE_PROTECT     :1;
    unsigned TMP_WRITE_PROTECT      :1;
    unsigned FILE_FORMAT            :2;
    unsigned CRC                    :7;
  //unsigned Always1                :1;
} csd;

typedef struct SdConfigurationRegister { //MSBit first
    /* value 0 is for physical layer spec 1.01-3.01 */
    unsigned SCR_STRUCTURE          :4;
    /* SD Memory Card - Spec. Version */
    unsigned SD_SPEC                :4;
    /* The data status is card vendor dependent */
    unsigned DATA_STAT_AFTER_ERASE  :1;
    /* CPRM Security Specification Version */
    unsigned SD_SECURITY            :3;
    /* DAT bus widths that are supported by the card */
    unsigned SD_BUS_WIDTHS          :4;
    /* Spec. Version 3.00 or higher */
    unsigned SD_SPEC3               :1;
    /* Extended Security support */
    unsigned EX_SECURITY            :4;
  //unsigned Reserved1              :9;
    /* new command support for newer cards */
    unsigned CMD_SUPPORT            :2;
  //uint32 Reserved2; // Reserved for manufacturer
} scr;

/*
 * Response Structures
 */

typedef struct CardStatusResponse { //MSBit first
    unsigned OUT_OF_RANGE           :1;
    unsigned ADDRESS_ERROR          :1;
    unsigned BLOCK_LEN_ERROR        :1;
    unsigned ERASE_SEQ_ERROR        :1;
    unsigned ERASE_PARAM            :1;
    unsigned WP_VIOLATION           :1;
    unsigned CARD_IS_LOCKED         :1;
    unsigned LOCK_UNLOCK_FAILED     :1;
    unsigned COM_CRC_ERROR          :1;
    unsigned ILLEGAL_COMMAND        :1;
    unsigned CARD_ECC_FAILED        :1;
    unsigned CC_ERROR               :1;
    unsigned ERROR                  :1;
  //unsigned Reserved1              :1;
  //unsigned Reserved2              :1; //DEFERRED_RESPONSE
    unsigned CSD_OVERWRITE          :1;
    unsigned WP_ERASE_SKIP          :1;
    unsigned CARD_ECC_DISABLED      :1;
    unsigned ERASE_RESET            :1;
    unsigned CURRENT_STATE          :4;
    unsigned READY_FOR_DATA         :1;
  //unsigned Reserved3              :2;
    unsigned APP_CMD                :1;
  //unsigned Reserved4              :1;
    unsigned AKE_SEQ_ERROR          :1;
  //unsigned Reserved5              :1;
  //unsigned Reserved6              :2;
} csr;

typedef struct SdStatusResponse { //MSBit first for Wide Width Data
    unsigned DAT_BUS_WIDTH          :2;
    unsigned SECURED_MODE           :1;
  //Reserved1                       :7; //Reserved for Security Functions
  //Reserved2                       :6;
    uint16 SD_CARD_TYPE;
    uint32 SIZE_OF_PROTECTED_AREA;
    uint8 SPEED_CLASS;
    uint8 PERFORMANCE_MOVE;
    unsigned AU_SIZE                :4;
  //unsigned Reserved3              :4;
    uint16 ERASE_SIZE;
    unsigned ERASE_TIMEOUT          :6;
    unsigned ERASE_OFFSET           :2;
    unsigned UHS_SPEED_GRADE        :4;
    unsigned UHS_AU_SIZE            :4;
  //uint8 Reserved4[10];
  //uint8 Reserved5[39]; //Reserved for Manufacturer
} ssr;

/* SDIO Card Structures, TBD
//CardCommonControlRegister
typedef struct CardCommonControlRegister {} cccr;

//FunctionBasicRegister
typedef struct FunctionBasicRegister {} fbr;

//CardInformationStructure
typedef struct CardInformationStructure {} cis;

/CodeStorageArea
typedef struct CodeStorageArea {} csa;
*/

class HardwareSDIO {
  public:
    icr ICR;
    ocr OCR;
    cid CID;
    csd CSD;
    rca RCA;
    scr SCR;
    ssr SSR;
    dsr DSR; // Default is 0x0404
    csr CSR;
    SDAppCommand appCmd;
    SDIOClockFrequency clkFreq;
    SDIOBlockSize blkSize;

    HardwareSDIO(void);
    /*----------------------------------------------------- public functions */
    void begin(SDIOClockFrequency);
    void begin(void);
    void end(void);
  //void read(uint32, uint32*);
    void read(uint32, uint32*, uint32);
  //void write(uint32, uint32*);
    void write(uint32, uint32*, uint32);
//protected:
    /*--------------------------------------- card register access functions */
    void getICR(uint32);
    void getOCR(uint32);
    void newRCA(void);
    void getCID(void);
    void getCSD(void);
    void setDSR(void);
    void getSCR(void);
    void getSSR(void);
    void getCSR(void);
    /*----------------------------------------- card register save functions */
    void convert(ocr*);
    void convert(csr*, uint32);
    void convert(csd*);
    void convert(cid*);
    void convert(rca*);
    /*------------------------------------------------ convenience functions */
    void idle(void);
    void initialization(void);
    void clockFreq(SDIOClockFrequency);
    void busMode(SDIOBusMode);
    void blockSize(SDIOBlockSize);
    void select(uint16);
    void select(void);
    void deselect(void);
    /*------------------------------------------------- basic data functions */
    void stop(void);
    void readBlock(uint32, uint32*);
    void writeBlock(uint32, uint32*);
  //private:
    /*---------------------------------------------------- command functions */
    void command(SDCommand, uint32);
    void command(SDAppCommand, uint32);
    void response(SDCommand);
    void response(SDAppCommand);
};

#endif