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
 * @author Marti Bolivar <mbolivar@leaflabs.com>
 * @brief Wirish SPI implementation.
 */

#include "HardwareSPI.h"

#include "timer.h"
#include "util.h"
#include "rcc.h"

#include "wirish.h"
#include "boards.h"


/*
 * Constructor
 */

HardwareSDIO::HardwareSDIO(void) {

}

/*
 * Public Members
 */

void HardwareSPI::begin(SPIFrequency frequency, uint32 bitOrder, uint32 mode) {
    if (mode >= 4) {
        ASSERT(0);
        return;
    }
    spi_cfg_flag end = bitOrder == MSBFIRST ? SPI_FRAME_MSB : SPI_FRAME_LSB;
    spi_mode m = (spi_mode)mode;
    enable_device(this->spi_d, true, frequency, end, m);
}

void HardwareSPI::end(void) {

}

/*
 * I/O
 */

void HardwareSPI::read(uint8 *buf, uint32 len) {
    uint32 rxed = 0;
    while (rxed < len) {
        while (!spi_is_rx_nonempty(this->spi_d))
            ;
        buf[rxed++] = (uint8)spi_rx_reg(this->spi_d);
    }
}

void HardwareSPI::write(const uint8 *data, uint32 length) {
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
void HardwareSDIO::sdio_gpio_init_cfg(SDIODataBusWidth data_bus_width) {
    switch (data_bus_width) {
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
    }
}

HardwareSDIO::sdio_card_id_process(void) {
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
void HardwareSDIO::sdio_send_cmd(SDIO_CmdInitTypeDef *SDIO_CmdInitStruct)
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
uint32 HardwareSDIO::ReadData(void)
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
