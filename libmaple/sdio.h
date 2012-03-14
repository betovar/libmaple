/******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2010 Perry Hung.
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
 * @file sdio.h
 * @author Brian E Tovar <betovar@leaflabs.com>
 * @brief SDIO interface support
 */

#ifndef _SDIO_H_
#define _SDIO_H_

#include "libmaple_types.h"
#include "rcc.h"
#include "nvic.h"
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif


/** SDIO register map type */
typedef struct sdio_reg_map {
    __io uint32 PWR;            ///< Power Control Register
    __io uint32 CLKCR;          ///< Clock Control Register
    __io uint32 ARG;            ///< Argument Register
    __io uint32 CMD;            ///< Command Register
    __io uint32 RESPCMD;        ///< Command Response Register
    __io uint32 RESP1;          ///< Response Register 1
    __io uint32 RESP2;          ///< Response Register 2
    __io uint32 RESP3;          ///< Response Register 3
    __io uint32 RESP4;          ///< Response Register 4
    __io uint32 DTIMER;         ///< Data Timer Register
    __io uint32 DLEN;           ///< Data Length Register
    __io uint32 DCTRL;          ///< Data Control Register
    __io uint32 DCOUNT;         ///< Data Count Register
    __io uint32 STA;            ///< Status Register
    __io uint32 ICR;            ///< Interrupt Clear Register
    __io uint32 MASK;           ///< Mask Register
    __io uint32 FIFOCNT;        ///< FIFO Count Register
    __io uint32 FIFO;           ///< Data FIFO Resgister
} sdio_reg_map;

/** SDIO register map base pointer */
#define SDIO_BASE                       ((struct sdio_reg_map*)0x40018000)
	
	
/*
 * Register bit definitions
 */
	
/* Power Control Register */
#define SDIO_PWR_PWRCTRL            0x3
	
/* Clock Control Register */
#define SDIO_CLKCR_HWFC_EN_BIT      14
#define SDIO_CLKCR_NEGEDGE_BIT      13
#define SDIO_CLKCR_BYPASS_BIT       10
#define SDIO_CLKCR_PWRSAV_BIT       9
#define SDIO_CLKCR_CLKLEN_BIT       8

#define SDIO_CLKCR_HWFC_EN          BIT(SDIO_CLKCR_HWFC_EN_BIT)
#define SDIO_CLKCR_NEGEDGE          BIT(SDIO_CLKCR_NEGEDGE_BIT)
#define SDIO_CLKCR_WIDBUS           (0x3 << 11)
#define SDIO_CLKCR_WIDEBUS_DEFAULT  (0x0 << 11)
#define SDIO_CLKCR_WIDEBUS_4WIDE    (0x1 << 11)
#define SDIO_CLKCR_WIDEBUS_8WIDE    (0x2 << 11)
#define SDIO_CLKCR_BYPASS           BIT(SDIO_CLKCR_BYPASS_BIT)
#define SDIO_CLKCR_PWRSAV           BIT(SDIO_CLKCR_PWRSAV_BIT)
#define SDIO_CLKCR_CLKLEN           BIT(SDIO_CLKCR_CLKLEN_BIT)
#define SDIO_CLKCR_CLKDIV           0xFF
	
/* Argument Register */
#define SDIO_ARG_CMDARG             0xFFFFFFFF
	
/* Command Register */
#define SDIO_CMD_ATACMD_BIT         14
#define SDIO_CMD_NIEN_BIT           13
#define SDIO_CMD_ENCMD_BIT          12
#define SDIO_CMD_SDIOS_BIT          11
#define SDIO_CMD_CPSMEN_BIT         10
#define SDIO_CMD_WAITPEND_BIT       9
#define SDIO_CMD_WAITINT_BIT        8

#define SDIO_CMD_ATACMD             BIT(SDIO_CMD_ATACMD_BIT)
#define SDIO_CMD_NIEN               BIT(SDIO_CMD_NIEN_BIT)
#define SDIO_CMD_ENCMD              BIT(SDIO_CMD_ENCMD_BIT)
#define SDIO_CMD_SDIOS              BIT(SDIO_CMD_SDIOS_BIT)
#define SDIO_CMD_CPSMEN             BIT(SDIO_CMD_CPSMEN_BIT)
#define SDIO_CMD_WAITPEND           BIT(SDIO_CMD_WAITPEND_BIT)
#define SDIO_CMD_WAITINT            BIT(SDIO_CMD_WAITINT_BIT)
#define SDIO_CMD_WAITRESP           (0x3 << 6)
#define SDIO_CMD_CMDINDEX           0x3F
	
/* Command Response Register */
#define SDIO_RESPCMD_RESPCMD        0x3F
	
/* Response Register 1 */
#define SDIO_RESP1_CARDSTATUS       0xFFFFFFFF
	
/* Response Register 2 */
#define SDIO_RESP2_CARDSTATUS       0xFFFFFFFF
	
/* Response Register 3 */
#define SDIO_RESP3_CARDSTATUS       0xFFFFFFFF
	
/* Response Register 4 */
#define SDIO_RESP4_CARDSTATUS       0xFFFFFFFF
	
/* Data Timer Register */
#define SDIO_DTIMER_DATATIME        0xFFFFFFFF
	
/* Data Length Register */
#define SDIO_DLEN_DATALENGTH        0x1FFFFFF
	
/* Data Control Register */
#define SDIO_DCTRL_SDIOEN_BIT       11
#define SDIO_DCTRL_RWMOD_BIT        10
#define SDIO_DCTRL_RWSTOP_BIT       9
#define SDIO_DCTRL_RWSTART_BIT      8
#define SDIO_DCTRL_DMAEN_BIT        3
#define SDIO_DCTRL_DTMODE_BIT       2
#define SDIO_DCTRL_DTDIR_BIT        1
#define SDIO_DCTRL_DTEN_BIT         0

#define SDIO_DCTRL_SDIOEN           BIT(SDIO_DCTRL_SDIOEN_BIT)
#define SDIO_DCTRL_RWMOD            BIT(SDIO_DCTRL_RWMOD_BIT)
#define SDIO_DCTRL_RWSTOP           BIT(SDIO_DCTRL_RWSTOP_BIT)
#define SDIO_DCTRL_RWSTART          BIT(SDIO_DCTRL_RWSTART_BIT)
#define SDIO_DCTRL_DBLOCKSIZE       (0xF << 4)
#define SDIO_DCTRL_DMAEN            BIT(SDIO_DCTRL_DMAEN_BIT)
#define SDIO_DCTRL_DTMODE           BIT(SDIO_DCTRL_DTMODE_BIT)
#define SDIO_DCTRL_DTDIR            BIT(SDIO_DCTRL_DTDIR_BIT)
#define SDIO_DCTRL_DTEN             BIT(SDIO_DCTRL_DTEN_BIT)
	
/* Data Count Register */
#define SDIO_DCOUNT_DATACOUNT       0x1FFFFFF
	
/* Status Register */
#define SDIO_STA_CEATAEND_BIT       23
#define SDIO_STA_SDIOIT_BIT         22
#define SDIO_STA_RXDAVL_BIT         21
#define SDIO_STA_TXDAVL_BIT         20
#define SDIO_STA_RXFIFOE_BIT        19
#define SDIO_STA_TXFIFOE_BIT        18
#define SDIO_STA_RXFIFOF_BIT        17
#define SDIO_STA_TXFIFOF_BIT        16
#define SDIO_STA_RXFIFOHF_BIT       15
#define SDIO_STA_TXFIFOHE_BIT       14
#define SDIO_STA_RXACT_BIT          13
#define SDIO_STA_TXACT_BIT          12
#define SDIO_STA_CMDACT_BIT         11
#define SDIO_STA_DBCKEND_BIT        10
#define SDIO_STA_STBITERR_BIT       9
#define SDIO_STA_DATAEND_BIT        8
#define SDIO_STA_CMDSENT_BIT        7
#define SDIO_STA_CMDREND_BIT        6
#define SDIO_STA_RXOVERR_BIT        5
#define SDIO_STA_TXUNDERR_BIT       4
#define SDIO_STA_DTIMEOUT_BIT       3
#define SDIO_STA_CTIMEOUT_BIT       2
#define SDIO_STA_DCRCFAIL_BIT       1
#define SDIO_STA_CCRCFAIL_BIT       0

#define SDIO_STA_CEATAEND           BIT(SDIO_STA_CEATAEND_BIT)
#define SDIO_STA_SDIOIT             BIT(SDIO_STA_SDIOIT_BIT)
#define SDIO_STA_RXDAVL             BIT(SDIO_STA_RXDAVL_BIT)
#define SDIO_STA_TXDAVL             BIT(SDIO_STA_TXDAVL_BIT)
#define SDIO_STA_RXFIFOE            BIT(SDIO_STA_RXFIFOE_BIT)
#define SDIO_STA_TXFIFOE            BIT(SDIO_STA_TXFIFOE_BIT)
#define SDIO_STA_RXFIFOF            BIT(SDIO_STA_RXFIFOF_BIT)
#define SDIO_STA_TXFIFOF            BIT(SDIO_STA_TXFIFOF_BIT)
#define SDIO_STA_RXFIFOHF           BIT(SDIO_STA_RXFIFOHF_BIT)
#define SDIO_STA_TXFIFOHE           BIT(SDIO_STA_TXFIFOHE_BIT)
#define SDIO_STA_RXACT              BIT(SDIO_STA_RXACT_BIT)
#define SDIO_STA_TXACT              BIT(SDIO_STA_TXACT_BIT)
#define SDIO_STA_CMDACT             BIT(SDIO_STA_CMDACT_BIT)
#define SDIO_STA_DBCKEND            BIT(SDIO_STA_DBCKEND_BIT)
#define SDIO_STA_STBITERR           BIT(SDIO_STA_STBITERR_BIT)
#define SDIO_STA_DATAEND            BIT(SDIO_STA_DATAEND_BIT)
#define SDIO_STA_CMDSENT            BIT(SDIO_STA_CMDSENT_BIT)
#define SDIO_STA_CMDREND            BIT(SDIO_STA_CMDREND_BIT)
#define SDIO_STA_RXOVERR            BIT(SDIO_STA_RXOVERR_BIT)
#define SDIO_STA_TXUNDERR           BIT(SDIO_STA_TXUNDERR_BIT)
#define SDIO_STA_DTIMEOUT           BIT(SDIO_STA_DTIMEOUT_BIT)
#define SDIO_STA_CTIMEOUT           BIT(SDIO_STA_CTIMEOUT_BIT)
#define SDIO_STA_DCRCFAIL           BIT(SDIO_STA_DCRCFAIL_BIT)
#define SDIO_STA_CCRCFAIL           BIT(SDIO_STA_CCRCFAIL_BIT)

/* Interrupt Clear Register */
#define SDIO_ICR_CEATAENDC_BIT      23
#define SDIO_ICR_SDIOITC_BIT        22
#define SDIO_ICR_DBCKENDC_BIT       10
#define SDIO_ICR_STBITERRC_BIT      9
#define SDIO_ICR_DATAENDC_BIT       8
#define SDIO_ICR_CMDSENTC_BIT       7
#define SDIO_ICR_CMDRENDC_BIT       6
#define SDIO_ICR_RXOVERRC_BIT       5
#define SDIO_ICR_TXUNDERRC_BIT      4
#define SDIO_ICR_DTIMEOUTC_BIT      3
#define SDIO_ICR_CTIMEOUTC_BIT      2
#define SDIO_ICR_DCRCFAILC_BIT      1
#define SDIO_ICR_CCRCFAILC_BIT      0
	
#define SDIO_ICR_CEATAENDC          BIT(SDIO_ICR_CEATAENDC_BIT)
#define SDIO_ICR_SDIOITC            BIT(SDIO_ICR_SDIOITC_BIT)
#define SDIO_ICR_DBCKENDC           BIT(SDIO_ICR_DBCKENDC_BIT)
#define SDIO_ICR_STBITERRC          BIT(SDIO_ICR_STBITERRC_BIT)
#define SDIO_ICR_DATAENDC           BIT(SDIO_ICR_DATAENDC_BIT)
#define SDIO_ICR_CMDSENTC           BIT(SDIO_ICR_CMDSENTC_BIT)
#define SDIO_ICR_CMDRENDC           BIT(SDIO_ICR_CMDRENDC_BIT)
#define SDIO_ICR_RXOVERRC           BIT(SDIO_ICR_RXOVERRC_BIT)
#define SDIO_ICR_TXUNDERRC          BIT(SDIO_ICR_TXUNDERRC_BIT)
#define SDIO_ICR_DTIMEOUTC          BIT(SDIO_ICR_DTIMEOUTC_BIT)
#define SDIO_ICR_CTIMEOUTC          BIT(SDIO_ICR_CTIMEOUTC_BIT)
#define SDIO_ICR_DCRCFAILC          BIT(SDIO_ICR_DCRCFAILC_BIT)
#define SDIO_ICR_CCRCFAILC          BIT(SDIO_ICR_CCRCFAILC_BIT)
	
/* Mask Register */
#define SDIO_MASK_CEATAENDIE_BIT    23
#define SDIO_MASK_SDIOITIE_BIT      22
#define SDIO_MASK_RXDAVLIE_BIT      21
#define SDIO_MASK_TXDAVLIE_BIT      20
#define SDIO_MASK_RXFIFOEIE_BIT     19
#define SDIO_MASK_TXFIFOEIE_BIT     18
#define SDIO_MASK_RXFIFOFIE_BIT     17
#define SDIO_MASK_TXFIFOFIE_BIT     16
#define SDIO_MASK_RXFIFOHFIE_BIT    15
#define SDIO_MASK_TXFIFOHFIE_BIT    14
#define SDIO_MASK_RXACTIE_BIT       13
#define SDIO_MASK_TXACTIE_BIT       12
#define SDIO_MASK_CMDACTIE_BIT      11
#define SDIO_MASK_DBCKENDIE_BIT     10
#define SDIO_MASK_STBITERRIE_BIT    9
#define SDIO_MASK_DATAENDIE_BIT     8
#define SDIO_MASK_CMDSENTIE_BIT     7
#define SDIO_MASK_CMDRENDIE_BIT     6
#define SDIO_MASK_RXOVERRIE_BIT     5
#define SDIO_MASK_TXUNDERRIE_BIT    4
#define SDIO_MASK_DTIMEOUTIE_BIT    3
#define SDIO_MASK_CTIMEOUTIE_BIT    2
s#define SDIO_MASK_DCRCFAILIE_BIT    1
#define SDIO_MASK_CCRCFAILIE_BIT    0
	
#define SDIO_MASK_CEATAENDIE        BIT(SDIO_MASK_CEATAENDIE_BIT)
#define SDIO_MASK_SDIOITIE          BIT(SDIO_MASK_SDIOITIE_BIT)
#define SDIO_MASK_RXDAVLIE          BIT(SDIO_MASK_RXDAVLIE_BIT)
#define SDIO_MASK_TXDAVLIE          BIT(SDIO_MASK_TXDAVLIE_BIT)
#define SDIO_MASK_RXFIFOEIE         BIT(SDIO_MASK_RXFIFOEIE_BIT)
#define SDIO_MASK_TXFIFOEIE         BIT(SDIO_MASK_TXFIFOEIE_BIT)
#define SDIO_MASK_RXFIFOFIE         BIT(SDIO_MASK_RXFIFOFIE_BIT)
#define SDIO_MASK_TXFIFOFIE         BIT(SDIO_MASK_TXFIFOFIE_BIT)
#define SDIO_MASK_RXFIFOHFIE        BIT(SDIO_MASK_RXFIFOHFIE_BIT)
#define SDIO_MASK_TXFIFOHFIE        BIT(SDIO_MASK_TXFIFOHFIE_BIT)
#define SDIO_MASK_RXACTIE           BIT(SDIO_MASK_RXACTIE_BIT)
#define SDIO_MASK_TXACTIE           BIT(SDIO_MASK_TXACTIE_BIT)
#define SDIO_MASK_CMDACTIE          BIT(SDIO_MASK_CMDACTIE_BIT)
#define SDIO_MASK_DBCKENDIE         BIT(SDIO_MASK_DBCKENDIE_BIT)
#define SDIO_MASK_STBITERRIE        BIT(SDIO_MASK_STBITERRIE_BIT)
#define SDIO_MASK_DATAENDIE         BIT(SDIO_MASK_DATAENDIE_BIT)
#define SDIO_MASK_CMDSENTIE         BIT(SDIO_MASK_CMDSENTIE_BIT)
#define SDIO_MASK_CMDRENDIE         BIT(SDIO_MASK_CMDRENDIE_BIT)
#define SDIO_MASK_RXOVERRIE         BIT(SDIO_MASK_RXOVERRIE_BIT)
#define SDIO_MASK_TXUNDERRIE        BIT(SDIO_MASK_TXUNDERRIE_BIT)
#define SDIO_MASK_DTIMEOUTIE        BIT(SDIO_MASK_DTIMEOUTIE_BIT)
#define SDIO_MASK_CTIMEOUTIE        BIT(SDIO_MASK_CTIMEOUTIE_BIT)
#define SDIO_MASK_DCRCFAILIE        BIT(SDIO_MASK_DCRCFAILIE_BIT)
#define SDIO_MASK_CCRCFAILIE        BIT(SDIO_MASK_CCRCFAILIE_BIT)
	
/* FIFO Count Register */
#define SDIO_FIFOCNT_FIFOCOUNT      0xFFFFFF

/* FIFO Register */
#define SDIO_FIFO_FIFODATA          0xFFFFFFFF

	
/*
 * SDIO Device
 */

/** SDIO device type */
typedef struct sdio_dev {
    sdio_reg_map *regs;         /**< Register map */
    rcc_clk_id clk_id;          /**< RCC clock information */
    nvic_irq_num irq_num;       /**< NVIC interrupt number */
} sdio_dev;

#ifdef STM32_HIGH_DENSITY
extern sdio_dev *SDIO;
#endif

	
/*
 * SDIO Functions
 */

void sdio_init(sdio_dev *dev);
void sdio_set_ck_freq(sdio_dev *dev);
void sdio_configure_gpio(sdio_dev *dev);

void sdio_peripheral_enable(sdio_dev *dev);
void sdio_peripheral_disable(sdio_dev *dev);

void sdio_configure_dma(sdio_dev *dev);

void sdio_tx_dma_enable(sdio_dev *dev);
void sdio_tx_dma_disable(sdio_dev *dev);

void sdio_rx_dma_enable(sdio_dev *dev);
void sdio_rx_dma_disable(sdio_dev *dev);

void sdio_broadcast_cmd(sdio_dev *dev);
void sdio_broadcast_cmd_wresponse(sdio_dev *dev);
void sdio_addr_cmd(sdio_dev *dev);
void sdio_addr_data_xfer_cmd(sdio_dev *dev);


#ifdef __cplusplus
}
#endif

#endif
