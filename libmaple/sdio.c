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
 * @brief Initialization SDIO clock control register
 * @param dev Device to initialize
 * @param clk_div clock divider factor to set the sdio_ck frequency
 */
void sdio_ck_init(sdio_dev *dev, uint8 clk_div) {
    /* HWFC_EN: Hardware Flow Control is Disabled */
    bb_peri_set_bit(&dev->regs->CLKCR, SDIO_CLKCR_HWFC_EN_BIT, 0);
    /* NEGEDGE: SDIO_CK generated on rising edge of SDIOCLK */
    bb_peri_set_bit(&dev->regs->CLKCR, SDIO_CLKCR_NEGEDGE_BIT, 0);
    /* WIDBUS: 1-bit bus mode during initialization */
    dev->regs->CLKCR &= ~SDIO_CLKCR_WIDEBUS;
    /* BYPASS: Clock divider bypass disabled */
    bb_peri_set_bit(&dev->regs->CLKCR, SDIO_CLKCR_BYPASS_BIT, 0);
    /* PWRSAV: Turn power save on by default */
    bb_peri_set_bit(&dev->regs->CLKCR, SDIO_CLKCR_PWRSAV_BIT, 1);
    /* CLKDIV: Clock Divide Factor, SDIO_CK = SDIOCLK/[CLKDIV+2] */
    bb_peri_set_bit(&dev->regs->CLKCR, SDIO_CLKCR_CLKDIV_BIT, 1);
    /* CLKEN: Clock is enabled */
    bb_peri_set_bit(&dev->regs->CLKCR, SDIO_CLKCR_CLKEN_BIT, 1);
}

/**
 * @brief 
 * @param dev 
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
 * @brief 
 * @param dev 
 */
void sdio_configure_gpio(sdio_dev *dev) {

}

/**
 * @brief Enable an SDIO peripheral
 * @param dev Device to enable
 */
void sdio_peripheral_enable(sdio_dev *dev) {
    //bb_peri_set_bit(&dev->regs->CR1, SPI_CR1_SPE_BIT, 1);
}

/**
 * @brief Disable an SDIO peripheral
 * @param dev Device to disable
 */
void sdio_peripheral_disable(sdio_dev *dev) {
    //bb_peri_set_bit(&dev->regs->CR1, SPI_CR1_SPE_BIT, 0);
}

/**
 * @brief 
 * @param dev 
 */
void sdio_configure_dma(sdio_dev *dev);


/**
 * @brief Enable DMA requests whenever the transmit buffer is empty
 * @param dev Device on which to enable TX DMA requests
 */
void sdio_tx_dma_enable(sdio_dev *dev) {
    //bb_peri_set_bit(&dev->regs->CR2, SPI_CR2_TXDMAEN_BIT, 1);
}

/**
 * @brief Disable DMA requests whenever the transmit buffer is empty
 * @param dev Device on which to disable TX DMA requests
 */
void sdio_tx_dma_disable(sdio_dev *dev) {
    //bb_peri_set_bit(&dev->regs->CR2, SPI_CR2_TXDMAEN_BIT, 0);
}

/**
 * @brief Enable DMA requests whenever the receive buffer is empty
 * @param dev Device on which to enable RX DMA requests
 */
void sdio_rx_dma_enable(sdio_dev *dev) {
    //bb_peri_set_bit(&dev->regs->CR2, SPI_CR2_RXDMAEN_BIT, 1);
}

/**
 * @brief Disable DMA requests whenever the receive buffer is empty
 * @param dev Device on which to disable RX DMA requests
 */
void sdio_rx_dma_disable(sdio_dev *dev) {
    //bb_peri_set_bit(&dev->regs->CR2, SPI_CR2_RXDMAEN_BIT, 0);
}

/**
 * @brief 
 * @param dev 
 */
void sdio_broadcast_cmd(sdio_dev *dev);

/**
 * @brief 
 * @param dev 
 */
void sdio_broadcast_cmd_wresponse(sdio_dev *dev);

/**
 * @brief 
 * @param dev 
 */
void sdio_addr_cmd(sdio_dev *dev);

/**
 * @brief 
 * @param dev 
 */
void sdio_addr_data_xfer_cmd(sdio_dev *dev);



/*
 * IRQ handlers (TODO)
 */
