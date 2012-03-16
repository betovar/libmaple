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
 * SDIO convenience functions
 */

/**
 * @brief Initialize and reset an SDIO device
 * @param dev Device to initialize and reset
 */
void sdio_init(sdio_dev *dev) {
    rcc_clk_enable(dev->clk_id);
    rcc_reset_dev(dev->clk_id);
}

/**
 * @brief Reset SDIO Device
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
 */
void sdio_set_ccr(sdio_dev *dev, uint32 ccr) {
    // Elminate stray bits in the reserved space
    dev->regs->CLKCR = (~SDIO_CLKCR_RESERVED &= ccr);
}

/**
 * @brief Set Clock Divisor in the Clock Control Register
 * @param dev SDIO
 * @param clk_div clock divider factor to set the sdio_ck frequency
 */
void sdio_cfg_clock(sdio_dev *dev, uint8 clk_div) {
    dev->regs->CCR &= ~SDIO_CCR_CLKDIV;
    dev->regs->CCR |= (uint32)clk_div;
}

/**
 * @brief Set the Data Control Register
 * @param dev SDIO Device
 * @param ccr Data Control Register Data 
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
    dev->regs->DCTRL = (~SDIO_DCTRL_RESERVED & dcr);
}

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
void sdio_post_cmd(sdio_dev *dev, uint8 cmd) {
    // Ignore possible WAITRESP data
    cmd &= SDIO_CMD_CMDINDEX;
    // Elminate stray bits in the reserved space
    dev->regs->CMD |= (~SDIO_CMD_RESERVED &= (uint32)cmd);
}

/**
 * @brief Get command response
 * @param dev SDIO Device 
 */
uint8 sdio_get_resp(sdio_dev *dev) {
    uint32 ret = dev->regs->RESPCMD;
    return (uint8)(~SDIO_RESPCMD_RESERVED & ret);
}

/**
 * @brief Set response timeout
 * @param dev SDIO Device 
 * @param timeout Timeout value for the data path state
 *        machine in card bus clock periods.
 */
void sdio_set_timeout(sdio_dev *dev, uint32 timeout) {
    dev->regs->RESPCMD = timeout;
}

/**
 * @brief Enable an SDIO peripheral
 * @param dev SDIO Device
 */
void sdio_peripheral_enable(sdio_dev *dev) {
    //bb_peri_set_bit(&dev->regs->, SPI_CR1_SPE_BIT, 1);
}

/**
 * @brief Disable an SDIO peripheral
 * @param dev SDIO Device
 */
void sdio_peripheral_disable(sdio_dev *dev) {
    //bb_peri_set_bit(&dev->regs->CR1, SPI_CR1_SPE_BIT, 0);
}

/**
 * @brief 
 * @param dev SDIO device
 */
void sdio_cfg_dma(sdio_dev *dev) {
    //dev->regs->DCTRL, SDIO_DCTRL_DMAEN
}


/**
 * @brief Enable DMA requests whenever the transmit buffer is empty
 * @param dev SDIO device
 */
void sdio_dma_enable(sdio_dev *dev) {
    //bb_peri_set_bit(&dev->regs->DCTRL, SDIO_DCTRL_DMAEN_BIT, 1);
}

/**
 * @brief Disable DMA requests whenever the transmit buffer is empty
 * @param dev SDIO device
 */
void sdio_dma_disable(sdio_dev *dev) {
    //bb_peri_set_bit(&dev->regs->DCTRL, SDIO_DCTRL_DMAEN_BIT, 1);
}

/**
 * @brief Post a Broadcast Command to the SDIO card
 * @param dev SDIO Device
 */
void sdio_broadcast_cmd(sdio_dev *dev, uint8 cmd) {
    dev->regs->CMD &= SDIO_CMD_
    dev->regs->CMD |= (uint32)cmd;
    bb_peri_set_bit(&dev->regs->CMD, SDIO_CMD_CMDEN, 1);
}

/**
 * @brief Broadcast Command with Response to the SDIO card
 * @param dev SDIO Device
 */
void sdio_broadcast_cmd_wresponse(sdio_dev *dev, uint8 cmd);

/**
 * @brief Addressed Command to the SDIO card
 * @param dev SDIO Device
 */
void sdio_addr_cmd(sdio_dev *dev, uint8 cmd);

/**
 * @brief Addressed Data Transfer Command to the SDIO card
 * @param dev SDIO Device
 */
void sdio_addr_data_xfer_cmd(sdio_dev *dev, uint8 cmd);


/*
 * IRQ handlers (TODO)
 */
