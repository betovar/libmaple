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
 * @file sdio.c
 * @author Brian E Tovar <betovar@leaflabs.com>
 * @brief SDIO interface support
 */

#include "sdio.h"
#include "gpio.h"
#include "bitband.h"


/*
 * SDIO device
 */

#ifdef STM32_HIGH_DENSITY
static sdio_dev sdio = {
    .regs     = SDIO_BASE,
    .clk_id   = RCC_SDIO,
    .irq_num  = NVIC_SDIO,
};
/** SDIO device */
sdio_dev *SDIO = &sdio;
#endif


/*
 * SDIO configure functions
 */

/**
 * @brief Initialize and reset an SDIO device
 * @param dev Device to initialize and reset
 */
void sdio_init(sdio_dev *dev) {
    rcc_clk_enable(dev->clk_id);
    rcc_reset_dev(dev->clk_id);
    dev->regs->POWER = SDIO_POWER_ON;
}

/**
 * @brief Reset an SDIO Device
 * @param dev SDIO Device
 */
void sdio_reset(sdio_dev *dev) {
    dev->regs->POWER  = 0x00000000;
    dev->regs->CLKCR  = 0x00000000;
    dev->regs->ARG    = 0x00000000;
    dev->regs->CMD    = 0x00000000;
    dev->regs->DTIMER = 0x00000000;
    dev->regs->DLEN   = 0x00000000;
    dev->regs->DCTRL  = 0x00000000;
    dev->regs->ICR    = 0x00C007FF;
    dev->regs->MASK   = 0x00000000;
}

/**
 * @brief Set the Clock Control Register
 * @param dev SDIO Device
 * @param ccr Clock Control Register Data
 * @note you should know what you're doing to use this
 */
void sdio_set_ccr(sdio_dev *dev, uint32 ccr) {
    // Elminate stray bits in the reserved space
    dev->regs->CLKCR = (~SDIO_CLKCR_RESERVED & ccr);
}

/**
 * @brief Configure the Command Path State Machine
 * @param dev SDIO Device
 * @param spc Register space to clear
 * @param val value to set in register
 */
void sdio_cfg_cpsm(sdio_dev *dev, uint32 spc, uint32 val) {
    dev->regs->CLKCR &= ~spc;
    // Elminate stray bits in the reserved space
    dev->regs->CLKCR |= (~SDIO_CLKCR_RESERVED & (spc & val));
}

/**
 * @brief Set Clock Divisor in the Clock Control Register
 * @param dev SDIO
 * @param clk_div clock divider factor to set the sdio_ck frequency
 */
void sdio_cfg_clock(sdio_dev *dev, uint8 clk_div) {

    /* CLKDIV: Set Clock Divider SDIOCLK/[CLKDIV+2]. */
    dev->regs->CLKCR &= ~SDIO_CLKCR_CLKDIV;
    dev->regs->CLKCR |= (uint32)clk_div;
}

/**
 * @brief Configure GPIO bit modes for use as an SDIO port's pins
 * @param width WIDBUS tag to configure pins for use as an SDIO card
 * @note 8-bit data bus width is only allowed for UHS-I cards
 * @note This assumes you're on a LeafLabs-style board
 *       (CYCLES_PER_MICROSECOND == 72, APB2 at 72MHz, APB1 at 36MHz).
 */
void sdio_cfg_bus(sdio_dev *dev, uint8 width) {
    switch (width) { //These gpios should be constant for the F1 line
    case 2:
        gpio_set_mode(GPIOC, 7, //SDIO_D7
                      GPIO_OUTPUT_PP);
        gpio_set_mode(GPIOC, 6, //SDIO_D6
                      GPIO_OUTPUT_PP);
        gpio_set_mode(GPIOB, 9, //SDIO_D5
                      GPIO_OUTPUT_PP);
        gpio_set_mode(GPIOB, 8, //SDIO_D4
                      GPIO_OUTPUT_PP);
    case 1:
        gpio_set_mode(GPIOC, 11, //SDIO_D3
                      GPIO_OUTPUT_PP);
        gpio_set_mode(GPIOC, 10, //SDIO_D2
                      GPIO_OUTPUT_PP);
    case 0:
        gpio_set_mode(GPIOC, 9, //SDIO_D1 
                      GPIO_OUTPUT_PP);
        gpio_set_mode(GPIOC, 8, //SDIO_D0
                      GPIO_OUTPUT_PP);
        gpio_set_mode(GPIOC, 12, //SDIO_CK
                      GPIO_OUTPUT_OD);
        gpio_set_mode(GPIOD, 2, //SDIO_CMD
                      GPIO_OUTPUT_PP);
        break;
    default:
        ASSERT(0);
    } //end of switch case
    if (width <= 1) {
        sdio_cfg_cpsm(dev, SDIO_CLKCR_WIDBUS, 
                      (width << SDIO_CLKCR_WIDBUS_BIT) );
    } else {
        ASSERT(0); //TODO[0.2.0] add support for UHS cards
    }
}

/**
 * @brief Set the Data Control Register
 * @param dev SDIO Device
 * @param dcr Data Control Register Data
 * @note you should know what you're doing to use this
 */
void sdio_set_dcr(sdio_dev *dev, uint32 dcr) {
    dev->regs->DCTRL = (~SDIO_DCTRL_RESERVED & dcr);
}

/**
 * @brief Configure the Data Path State Machine through the 
 *        Data Control Register
 * @param dev SDIO Device 
 * @param dcr Data Control Register Data
 * @note  A data transfer must be written to the data timer register
 *        and the data length register before being written to the
 *        data control register.
 */
void sdio_cfg_dpsm(sdio_dev *dev, uint32 dcr) {
    dev->regs->DLEN = 1; //FIXME
    dev->regs->DCTRL = (~SDIO_DCTRL_RESERVED & dcr);
}

/*
 * SDIO hardware functions
 */

/**
 * @brief Enable an SDIO peripheral
 * @param dev SDIO Device
 */
void sdio_peripheral_enable(sdio_dev *dev) {
    bb_peri_set_bit(&dev->regs->DCTRL, SDIO_DCTRL_SDIOEN_BIT, 1);
}

/**
 * @brief Disable an SDIO peripheral
 * @param dev SDIO Device
 */
void sdio_peripheral_disable(sdio_dev *dev) {
    bb_peri_set_bit(&dev->regs->DCTRL, SDIO_DCTRL_SDIOEN_BIT, 0);
}

/**
 * @brief Enable DMA requests whenever the transmit buffer is empty
 * @param dev SDIO device
 */
void sdio_dma_enable(sdio_dev *dev) {
    bb_peri_set_bit(&dev->regs->DCTRL, SDIO_DCTRL_DMAEN_BIT, 1);
}

/**
 * @brief Disable DMA requests whenever the transmit buffer is empty
 * @param dev SDIO device
 */
void sdio_dma_disable(sdio_dev *dev) {
    bb_peri_set_bit(&dev->regs->DCTRL, SDIO_DCTRL_DMAEN_BIT, 0);
}

/**
 * @brief 
 * @param dev SDIO device
 */
void sdio_cfg_dma(sdio_dev *dev) {
    //other things go here
    //b_peri_set_bit(&dev->regs->DCTRL, SDIO_DCTRL_DMAEN_BIT, 1);
}

/*
 * SDIO command functions
 */

/**
 * @brief Load argument into SDIO Argument Register
 * @param dev SDIO Device
 * @param arg Argument Data
 */
void sdio_load_arg(sdio_dev *dev, uint32 arg) {
    dev->regs->ARG = arg;
}

/**
 * @brief Send command to external card
 * @param dev SDIO Device
 * @param cmd SDIO Command to send 
 */
void sdio_send_cmd(sdio_dev *dev, uint8 cmd) {
    // copy register
    uint32 temp = dev->regs->CMD;
    // clear cmd space
    temp &= !(SDIO_CMD_WAITRESP & SDIO_CMD_CMDINDEX);
    temp |= (uint32)cmd;
    // load command register
    dev->regs->CMD = temp;
}

/**
 * @brief Get last sent command
 * @param dev SDIO Device 
 */
uint8 sdio_get_cmd(sdio_dev *dev) {
    uint32 resp = dev->regs->RESPCMD;
    return (uint8)(~SDIO_RESPCMD_RESERVED & resp);
}

/**
 * @brief Get short response
 * @param dev SDIO Device
 * @retval The 32-bit short response
 */
uint32 sdio_get_resp_short(sdio_dev *dev) {
    return dev->regs->RESP1;
}

/**
 * @brief Get long response
 * @param dev SDIO Device 
 * @param buf Pointer to 32-bit response buffer
 * @retval None
 */
void sdio_get_resp_long(sdio_dev *dev, uint32 *buf) {
     buf[0] = dev->regs->RESP1;
     buf[1] = dev->regs->RESP2;
     buf[2] = dev->regs->RESP3;
     buf[3] = dev->regs->RESP4;
}

/**
 * @brief Post a Broadcast Command to all SDIO cards
 * @param dev SDIO Device
 */
void sdio_broadcast_cmd(sdio_dev *dev, uint8 cmd) {
    dev->regs->CMD &= ~SDIO_CMD_CMDINDEX;
    dev->regs->CMD |= (uint32)cmd;
    //bb_peri_set_bit(&dev->regs->CMD, SDIO_CMD_CMDEN, 1);
}

/**
 * @brief Broadcast Command with Response to all SDIO cards
 * @param dev SDIO Device
 */
void sdio_broadcast_cmd_wresponse(sdio_dev *dev, uint8 cmd) {   
}

/**
 * @brief Addressed Command to the SDIO card
 * @param dev SDIO Device
 */
void sdio_addr_cmd(sdio_dev *dev, uint8 cmd) {
}

/**
 * @brief Addressed Data Transfer Command to the SDIO card
 * @param dev SDIO Device
 */
void sdio_addr_data_xfer_cmd(sdio_dev *dev, uint8 cmd) {
}

/*
 * SDIO status functions
 */

/**
 * @brief 
 * @param dev SDIO Device
 */
uint32 sdio_is_rx_data_aval(sdio_dev *dev) {
    return bb_peri_get_bit(&dev->regs->STA, SDIO_STA_RXDAVL_BIT);
}

/**
 * @brief 
 * @param dev SDIO Device
 */
uint32 sdio_is_tx_data_aval(sdio_dev *dev) {
    return bb_peri_get_bit(&dev->regs->STA, SDIO_STA_TXDAVL_BIT);
}

/**
 * @brief 
 * @param dev SDIO Device
 */
uint32 sdio_is_rx_act(sdio_dev *dev) {
    return bb_peri_get_bit(&dev->regs->STA, SDIO_STA_RXACT_BIT);
}

/**
 * @brief 
 * @param dev SDIO Device
 */
uint32 sdio_is_tx_act(sdio_dev *dev) {
    return bb_peri_get_bit(&dev->regs->STA, SDIO_STA_TXACT);
}

/**
 * @brief 
 * @param dev SDIO Device
 */
uint32 sdio_is_xfer_in_prog(sdio_dev *dev) {
    return bb_peri_get_bit(&dev->regs->STA, SDIO_STA_CMDACT);
}

/*
 * SDIO auxiliary functions
 */

/**
  * @brief  Read one data word from Rx FIFO.
  * @param  None
  * @retval Data received
  */
uint32 sdio_read_data(sdio_dev *dev) {
    return dev->regs->FIFO;
}

/**
  * @brief  Write one data word to Tx FIFO.
  * @param  Data: 32-bit data word to write.
  * @retval None
  */
void sdio_write_data(sdio_dev *dev, uint32 data) {
    dev->regs->FIFO = data;
}

/**
 * @brief Set response timeout
 * @param dev SDIO Device 
 * @param timeout Timeout value for the data path state
 *        machine in card bus clock periods.
 */
void sdio_set_timeout(sdio_dev *dev, uint32 timeout) {
    dev->regs->DTIMER = timeout;
}

/**
  * @brief  Part of the data path state machine, this (read-only)register
  *         loads the value from the data length register and decrements
  *         until 0.
  * @param  dev SDIO Device
  * @retval Number of remaining data bytes to be transferred 
  */
uint32 sdio_get_data_count(sdio_dev *dev) {
    return dev->regs->DCOUNT;
}

/**
  * @brief  Returns the number of words left to be written to or read from FIFO
  * @param  None
  * @retval Remaining number of words.
  */
uint32 sdio_get_fifo_count(sdio_dev *dev) {
    return dev->regs->FIFOCNT;
}

/*
 * SDIO interrupt functions
 */

/**
  * @brief  Clears the SDIO's pending flags.
  * @param  SDIO_FLAG: specifies the flag to clear.  
  *   This parameter can be one or a combination of the following values:
  *     @arg SDIO_FLAG_CCRCFAIL: Command response received (CRC check failed)
  *     @arg SDIO_FLAG_DCRCFAIL: Data block sent/received (CRC check failed)
  *     @arg SDIO_FLAG_CTIMEOUT: Command response timeout
  *     @arg SDIO_FLAG_DTIMEOUT: Data timeout
  *     @arg SDIO_FLAG_TXUNDERR: Transmit FIFO underrun error
  *     @arg SDIO_FLAG_RXOVERR:  Received FIFO overrun error
  *     @arg SDIO_FLAG_CMDREND:  Command response received (CRC check passed)
  *     @arg SDIO_FLAG_CMDSENT:  Command sent (no response required)
  *     @arg SDIO_FLAG_DATAEND:  Data end (data counter, SDIDCOUNT, is zero)
  *     @arg SDIO_FLAG_STBITERR: Start bit not detected on all data signals in wide 
  *                              bus mode
  *     @arg SDIO_FLAG_DBCKEND:  Data block sent/received (CRC check passed)
  *     @arg SDIO_FLAG_SDIOIT:   SD I/O interrupt received
  *     @arg SDIO_FLAG_CEATAEND: CE-ATA command completion signal received for CMD61
  * @retval None
  */
void sdio_clear_flag(sdio_dev *dev, uint32 flag) { 
    dev->regs->ICR |= flag;
}

/**
  * @brief  Checks whether the specified SDIO interrupt has occurred or not.
  * @param  SDIO_IT: specifies the SDIO interrupt source to check. 
  *   This parameter can be one of the following values:
  *   @arg SDIO_IT_CEATAEND: CE-ATA command completion signal received for
  *                          CMD61 interrupt
  *   @arg SDIO_IT_SDIOIT:   SD I/O interrupt received interrupt
  *   @arg SDIO_IT_RXDAVL:   Data available in receive FIFO interrupt
  *   @arg SDIO_IT_TXDAVL:   Data available in transmit FIFO interrupt
  *   @arg SDIO_IT_RXFIFOE:  Receive FIFO empty interrupt
  *   @arg SDIO_IT_TXFIFOE:  Transmit FIFO empty interrupt
  *   @arg SDIO_IT_RXFIFOF:  Receive FIFO full interrupt
  *   @arg SDIO_IT_TXFIFOF:  Transmit FIFO full interrupt
  *   @arg SDIO_IT_RXFIFOHF: Receive FIFO Half Full interrupt
  *   @arg SDIO_IT_TXFIFOHE: Transmit FIFO Half Empty interrupt
  *   @arg SDIO_IT_RXACT:    Data receive in progress interrupt
  *   @arg SDIO_IT_TXACT:    Data transmit in progress interrupt
  *   @arg SDIO_IT_CMDACT:   Command transfer in progress interrupt
  *   @arg SDIO_IT_DBCKEND:  Data block sent/received (CRC check passed)
  *                          interrupt
  *   @arg SDIO_IT_STBITERR: Start bit not detected on all data signals in wide 
  *                          bus mode interrupt
  *   @arg SDIO_IT_DATAEND:  Data end (data counter, SDIDCOUNT, is zero)
  *                          interrupt
  *   @arg SDIO_IT_CMDSENT:  Command sent (no response required) interrupt
  *   @arg SDIO_IT_CMDREND:  Command response received (CRC check passed)
  *                          interrupt
  *   @arg SDIO_IT_RXOVERR:  Received FIFO overrun error interrupt
  *   @arg SDIO_IT_TXUNDERR: Transmit FIFO underrun error interrupt
  *   @arg SDIO_IT_DTIMEOUT: Data timeout interrupt
  *   @arg SDIO_IT_CTIMEOUT: Command response timeout interrupt
  *   @arg SDIO_IT_DCRCFAIL: Data block sent/received (CRC check failed)
  *                          interrupt
  *   @arg SDIO_IT_CCRCFAIL: Command response received (CRC check failed)
  *                          interrupt
  * @retval status of the interrupt
  */
uint8 sdio_get_status(sdio_dev *dev, uint32 rupt) { 
    uint8 status;
    if (dev->regs->STA & rupt) {
        status = 1;
    } else {
        status = 0;
    }
    return status;
}
/**
  * @brief  Clears the SDIO's interrupt pending bits.
  * @param  pend: specifies the interrupt pending bit to clear. 
  *   This parameter can be one or a combination of the following values:
  *   @arg SDIO_ICR_CEATAENDC: CE-ATA command completion signal received for
  *                            CMD61
  *   @arg SDIO_ICR_SDIOITC:   SD I/O interrupt received interrupt
  *   @arg SDIO_ICR_STBITERRC: Start bit not detected on all data signals in
  *                            wide bus mode interrupt
  *   @arg SDIO_ICR_DATAENDC:  Data end (data counter, SDIDCOUNT, is zero)
  *                            interrupt
  *   @arg SDIO_ICR_CMDSENTC:  Command sent (no response required) interrupt
  *   @arg SDIO_ICR_CMDRENDC:  Command response received (CRC check passed)
  *                            interrupt
  *   @arg SDIO_ICR_RXOVERRC:  Received FIFO overrun error interrupt
  *   @arg SDIO_ICR_TXUNDERRC: Transmit FIFO underrun error interrupt
  *   @arg SDIO_ICR_DTIMEOUTC: Data timeout interrupt
  *   @arg SDIO_ICR_CTIMEOUTC: Command response timeout interrupt
  *   @arg SDIO_ICR_DCRCFAILC: Data block sent/received (CRC check failed)
  *                            interrupt
  *   @arg SDIO_IT_CCRCFAILC:  Command response received (CRC check failed)
  *                            interrupt
  * @retval None
  */
void sdio_clear_pending(sdio_dev *dev, uint32 pend) {
    dev->regs->ICR = pend;
}
