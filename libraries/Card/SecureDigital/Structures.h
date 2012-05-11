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
 * @file Registers.h
 * @author Brian E Tovar <betovar@leaflabs.com>
 * @breif These typedefs define card register data and properities
 *        Most of this information is taken from the Part 1 Physical Layer
 *        Specification Verson 3.01
 */

#ifndef _SD_STRUCTURES_H_
#define _SD_STRUCTURES_H_

/**
 * Register Structures
 */

typedef struct OperationConditionsRegister {
    /** VDD Voltage Window: 2.7v - 3.6v */
    unsigned VOLTAGE_WINDOW         :24;
    /** Switch to 1.8v Accepted:
     *  Only UHS-I card supports this bit */
    unsigned S18A                   :1;
    unsigned Reserved1              :5;
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
typedef struct RelativeCardAddress {
    uint8 Reserved2;
    uint8 Reserved1;
    uint16 RCA;
}__attribute__((packed)) rca;

// DriverStageRegister
typedef uint16 dsr; // Optional

typedef struct CardSpecificDataV1 {
    unsigned CSD_STRUCTURE          :2;
    unsigned Reserved1              :6;
    uint8 TAAC;
    uint8 NSAC;
    uint8 TRAN_SPEED;
    unsigned CCC                    :12;
    unsigned READ_BL_LEN            :4;
    unsigned READ_BL_PARTIAL        :1;
    unsigned WRITE_BLK_MISALIGN     :1;
    unsigned READ_BLK_MISALIGN      :1;
    unsigned DSR_IMP                :1;
    unsigned Reserved2              :2;
    unsigned C_SIZE                 :12;
    unsigned VDD_R_CURR_MIN         :3;
    unsigned VDD_R_CURR_MAX         :3;
    unsigned VDD_W_CURR_MIN         :3;
    unsigned VDD_W_CURR_MAX         :3;
    unsigned C_SIZE_MULT            :3;
    unsigned ERASE_BLK_EN           :1;
    unsigned SECTOR_SIZE            :7;
    unsigned WP_GRP_SIZE            :7;
    unsigned WP_GRP_ENABLE          :1;
    unsigned Reserved3              :2;
    unsigned R2W_FACTOR             :3;
    unsigned WRITE_BL_LEN           :4;
    unsigned WRITE_BL_PARTIAL       :1;
    unsigned Reserved4              :5;
    unsigned FILE_FORMAT_GRP        :1;
    unsigned COPY                   :1;
    unsigned PERM_WRITE_PROTECT     :1;
    unsigned TMP_WRITE_PROTECT      :1;
    unsigned FILE_FORMAT            :2;
    unsigned Reserved5              :2;
    unsigned CRC                    :7;
    unsigned Always1                :1;
}__attribute__((packed)) csdV1;

typedef struct CardSpecificDataV2 {
    unsigned CSD_STRUCTURE          :2;
    unsigned Reserved1              :6;
    uint8 TAAC;
    uint8 NSAC;
    uint8 TRAN_SPEED;
    unsigned CCC                    :12;
    unsigned READ_BL_LEN            :4;
    unsigned READ_BL_PARTIAL        :1;
    unsigned WRITE_BLK_MISALIGN     :1;
    unsigned READ_BLK_MISALIGN      :1;
    unsigned DSR_IMP                :1;
    unsigned Reserved2              :6;
    unsigned C_SIZE                 :22;
    unsigned Reserved3              :1;
    unsigned ERASE_BLK_EN           :1;
    unsigned SECTOR_SIZE            :7;
    unsigned WP_GRP_SIZE            :7;
    unsigned WP_GRP_ENABLE          :1;
    unsigned Reserved4              :2;
    unsigned R2W_FACTOR             :3;
    unsigned WRITE_BL_LEN           :4;
    unsigned WRITE_BL_PARTIAL       :1;
    unsigned Reserved5              :5;
    unsigned FILE_FORMAT_GRP        :1;
    unsigned COPY                   :1;
    unsigned PERM_WRITE_PROTECT     :1;
    unsigned TMP_WRITE_PROTECT      :1;
    unsigned FILE_FORMAT            :2;
    unsigned Reserved6              :2;
    unsigned CRC                    :7;
    unsigned Always1                :1;
}__attribute__((packed)) csdV2;

typedef enum CsdType {
    CSD_UNDEFINED = 0,
    CSD_VERSION1  = 1,
    CSD_VERSION2  = 2
} CsdType;

typedef struct CardSpecificData {
    union {
        csdV1 V1;
        csdV2 V2;
    };
    CsdType version;
}__attribute__((packed)) csd;

typedef struct SdConfigurationRegister {
    /** value 0 is for physical layer spec 1.01-3.01 */
    unsigned SCR_STRUCTURE          :4;
    /**  */
    unsigned SD_SPEC                :4;
    /** The data status is card vendor dependent */
    unsigned DATA_STAT_AFTER_ERASE  :1;
    /** CPRM Security Specification Version */
    unsigned SD_SECURITY            :3;
    /** DAT bus widths that are supported by the card */
    unsigned SD_BUS_WIDTHS          :4;
    /**  */
    unsigned SD_SPEC3               :1;
    /** Extended Security support */
    unsigned EX_SECURITY            :4;
    unsigned Reserved1              :9;
    /** new command support for newer cards */
    unsigned CMD_SUPPORT            :2;
    uint32 Reserved2;
}__attribute__((packed)) scr;

/**
 * Response Structures
 */

typedef struct CardStatusResponse {
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
    unsigned ADDRESS_MISALIGN       :1;
    unsigned ADDRESS_OUT_OF_RANGE   :1;
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

typedef struct InterfaceConditionResponse {
    unsigned CHECK_PATTERN          :8;
    unsigned VOLTAGE_ACCEPTED       :4;
    unsigned Reserved1              :20;
}__attribute__((packed)) icr;

#endif

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