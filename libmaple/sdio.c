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
 * SDIO convenience routines
 */

/**
 * @brief Configure GPIO bit modes for use as an SDIO port's pins
 * @param data_bus_width Enum to configure pins for use as an SDIO card
 */
void sdio_gpio_cfg(uint8 data_bus_width) {
    switch (data_bus_width) {
    case 2:
        // 8-bit data bus width not implemented on maple native
        gpio_set_mode(BOARD_SDIO_D7_DEV, BOARD_SDIO_D7_BIT, GPIO_AF_OUTPUT_PP);
        gpio_set_mode(BOARD_SDIO_D6_DEV, BOARD_SDIO_D6_BIT, GPIO_AF_OUTPUT_PP);
        gpio_set_mode(BOARD_SDIO_D5_DEV, BOARD_SDIO_D5_BIT, GPIO_AF_OUTPUT_PP);
        gpio_set_mode(BOARD_SDIO_D4_DEV, BOARD_SDIO_D4_BIT, GPIO_AF_OUTPUT_PP);
    case 1:
        gpio_set_mode(BOARD_SDIO_D3_DEV, BOARD_SDIO_D3_BIT, GPIO_AF_OUTPUT_PP);
        gpio_set_mode(BOARD_SDIO_D2_BIT, BOARD_SDIO_D2_BIT, GPIO_AF_OUTPUT_PP);
    case 0:
        gpio_set_mode(BOARD_SDIO_D1_BIT, BOARD_SDIO_D1_BIT, GPIO_AF_OUTPUT_PP);
        gpio_set_mode(BOARD_SDIO_D0_DEV, BOARD_SDIO_D0_BIT, GPIO_INPUT_FLOATING);
        gpio_set_mode(BOARD_SDIO_CK_DEV,BOARD_SDIO_CK_BIT, GPIO_AF_OUTPUT_OD);
        gpio_set_mode(BOARD_SDIO_CMD_DEV, BOARD_SDIO_CMD_BIT, GPIO_AF_OUTPUT_PP);
        break;
    default:
        ASSERT(0);
    }
}

/**
 * @brief Initialize and reset an SDIO device
 * @param dev Device to initialize and reset
 */
void sdio_init(sdio_dev *dev) {
    rcc_clk_enable(dev->clk_id);
    rcc_reset_dev(dev->clk_id);
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

/*
 * IRQ handlers (TODO)
 */
