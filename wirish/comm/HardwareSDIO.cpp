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
 * @author Brian E Tovar <betovar@leaflabs.com>
 * @brief Wirish SDIO implementation. 
 */

#include "HardwareSDIO.h"

#include "sdio.h"
#include "util.h"
#include "rcc.h"

#include "wirish.h"
//#include "boards.h"


/*
 * Constructor
 */

HardwareSDIO::HardwareSDIO(void) {
    sdio_peripheral_enable();
}

/*
 * Public Members
 */

void HardwareSDIO::begin(SDIOFrequency freq,
                         SDIODataBusSpeed speed,
                         SDIODataBusWidth mode) {
    //only one speed mode supported at this time
    if (speed != SDIO_DBS_DEFAULT) { 
        ASSERT(0);
        return;
    }
    cfgClock(freq);
    cfgDataBus(speed, mode);
    sdio_peripheral_enable();
}

void HardwareSDIO::end(void) {
    ASSERT(1);
}

/*
 * I/O
 */

void HardwareSDIO::read(uint8 *buf, uint32 len) {
    uint32 rxed = 0;
    while (rxed < len) {
        while (!spi_is_rx_nonempty(this->spi_d))
            ;
        buf[rxed++] = (uint8)spi_rx_reg(this->spi_d);
    }
}

void HardwareSDIO::write(const uint8 *data, uint32 length) {
    uint32 txed = 0;
    while (txed < length) {
        txed += spi_tx(this->spi_d, data + txed, length - txed);
    }
}

/*
 * Auxiliary functions
 */

/**
 * @brief Configure GPIO bit modes for use as an SDIO port's pins
 * @param data_bus_width Enum to configure pins for use as an SDIO card
 * @note 8-bit data bus width not implemented on maple as of March 2012
 */
void HardwareSDIO::cfgClock(SDIODataBusWidth width) {
    /* HWFC_EN: Hardware Flow Control is enabled */
    bb_peri_set_bit(&dev->regs->CLKCR, SDIO_CLKCR_HWFC_EN_BIT, 1);
    /* NEGEDGE: SDIO_CK generated on rising edge of SDIOCLK */
    bb_peri_set_bit(&dev->regs->CLKCR, SDIO_CLKCR_NEGEDGE_BIT, 0);
    /* WIDBUS: 1-bit bus mode during initialization */
    dev->regs->CLKCR &= ~SDIO_CLKCR_WIDEBUS;
    /* BYPASS: Clock divider bypass disabled */
    bb_peri_set_bit(&dev->regs->CLKCR, SDIO_CLKCR_BYPASS_BIT, 0);
    /* PWRSAV: Turn power save on by default */
    bb_peri_set_bit(&dev->regs->CLKCR, SDIO_CLKCR_PWRSAV_BIT, 1);
    /* CLKEN: Clock is enabled */
    bb_peri_set_bit(&dev->regs->CLKCR, SDIO_CLKCR_CLKEN_BIT, 1);
}

/**
 * @brief Configure GPIO bit modes for use as an SDIO port's pins
 * @param data_bus_width Enum to configure pins for use as an SDIO card
 * @note 8-bit data bus width not implemented on maple as of March 2012
 */
void HardwareSDIO::cfgGPIO(SDIODataBusWidth width) {
    switch (width) {
    case SDIO_DBW_8:
        gpio_set_mode(BOARD_SDIO_D7_DEV,
                      BOARD_SDIO_D7_BIT, 
                      GPIO_AF_OUTPUT_PP);
        gpio_set_mode(BOARD_SDIO_D6_DEV,
                      BOARD_SDIO_D6_BIT, 
                      GPIO_AF_OUTPUT_PP);
        gpio_set_mode(BOARD_SDIO_D5_DEV,
                      BOARD_SDIO_D5_BIT, 
                      GPIO_AF_OUTPUT_PP);
        gpio_set_mode(BOARD_SDIO_D4_DEV,
                      BOARD_SDIO_D4_BIT, 
                      GPIO_AF_OUTPUT_PP);
    case SDIO_DBW_4:
        gpio_set_mode(BOARD_SDIO_D3_DEV,
                      BOARD_SDIO_D3_BIT, 
                      GPIO_AF_OUTPUT_PP);
        gpio_set_mode(BOARD_SDIO_D2_BIT,
                      BOARD_SDIO_D2_BIT, 
                      GPIO_AF_OUTPUT_PP);
    case SDIO_DBW_0:
        gpio_set_mode(BOARD_SDIO_D1_BIT,
                      BOARD_SDIO_D1_BIT, 
                      GPIO_AF_OUTPUT_PP);
        gpio_set_mode(BOARD_SDIO_D0_DEV,
                      BOARD_SDIO_D0_BIT, 
                      GPIO_INPUT_FLOATING);
        gpio_set_mode(BOARD_SDIO_CK_DEV,
                      BOARD_SDIO_CK_BIT,
                      GPIO_AF_OUTPUT_OD);
        gpio_set_mode(BOARD_SDIO_CMD_DEV, 
                      BOARD_SDIO_CMD_BIT, 
                      GPIO_AF_OUTPUT_PP);
        break;
    default:
        ASSERT(0);
    } //end of switch case
}

HardwareSDIO::card_identification_process(void) {
    //activate bus
    //host broadcasts SD_APP_OP_COND
    //card resp: ocr
    //place incompatible cards to inactive state
    //host broadcasts ALL_SEND_COND 
    //card resp: cid numbers
    //host issues: SET_RELATIVE_ADDR (rca)
    //
}

/**
  * @brief  Initializes the SDIO Command according to the specified 
  *   parameters in the SDIO_CmdInitStruct and send the command.
  * @param SDIO_CmdInitStruct : pointer to a SDIO_CmdInitTypeDef 
  *   structure that contains the configuration information 
  *   for the SDIO command.
  * @retval : None
  */
void HardwareSDIO::send_command(SDIO_CmdInitTypeDef *SDIO_CmdInitStruct)
{
  uint32_t tmpreg = 0;
  
  /* Check the parameters */
  assert_param(IS_SDIO_CMD_INDEX(SDIO_CmdInitStruct->SDIO_CmdIndex));
  assert_param(IS_SDIO_RESPONSE(SDIO_CmdInitStruct->SDIO_Response));
  assert_param(IS_SDIO_WAIT(SDIO_CmdInitStruct->SDIO_Wait));
  assert_param(IS_SDIO_CPSM(SDIO_CmdInitStruct->SDIO_CPSM));
  
/*---------------------------- SDIO ARG Configuration ------------------------*/
  /* Set the SDIO Argument value */
  SDIO->ARG = SDIO_CmdInitStruct->SDIO_Argument;
  
/*---------------------------- SDIO CMD Configuration ------------------------*/  
  /* Get the SDIO CMD value */
  tmpreg = SDIO->CMD;
  /* Clear CMDINDEX, WAITRESP, WAITINT, WAITPEND, CPSMEN bits */
  tmpreg &= CMD_CLEAR_MASK;
  /* Set CMDINDEX bits according to SDIO_CmdIndex value */
  /* Set WAITRESP bits according to SDIO_Response value */
  /* Set WAITINT and WAITPEND bits according to SDIO_Wait value */
  /* Set CPSMEN bits according to SDIO_CPSM value */
  tmpreg |= (uint32_t)SDIO_CmdInitStruct->SDIO_CmdIndex | SDIO_CmdInitStruct->SDIO_Response
           | SDIO_CmdInitStruct->SDIO_Wait | SDIO_CmdInitStruct->SDIO_CPSM;
  
  /* Write to SDIO CMD */
  SDIO->CMD = tmpreg;
}

/**
  * @brief  Returns number of remaining data bytes to be transferred.
  * @param  None
  * @retval : Number of remaining data bytes to be transferred
  */
uint32 HardwareSDIO::GetDataCounter(void)
{ 
  return SDIO->DCOUNT;
}

/**
  * @brief  Read one data word from Rx FIFO.
  * @param  None
  * @retval : Data received
  */
uint32 HardwareSDIO::read_data(void)
{ 
  return SDIO->FIFO;
}

/**
  * @brief  Write one data word to Tx FIFO.
  * @param Data: 32-bit data word to write.
  * @retval : None
  */
void HardwareSDIO::WriteData(uint32_t Data)
{ 
  SDIO->FIFO = Data;
}

/**
  * @brief  Returns the number of words left to be written to or read
  *   from FIFO.	
  * @param  None
  * @retval : Remaining number of words.
  */
uint32 HardwareSDIO::GetFIFOCount(void)
{ 
  return SDIO->FIFOCNT;
}


