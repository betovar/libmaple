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
#include "timer.h"
#include "delay.h"
#include "bitband.h"
#include "dma.h"

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
  //nvic_irq_enable(dev->irq_num);
}

/**
 * @brief Reset an SDIO Device
 * @param dev SDIO Device
 */
void sdio_reset(sdio_dev *dev) {
  //nvic_irq_disable(dev->irq_num);
    rcc_reset_dev(dev->clk_id);
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
 * @brief Power on the SDIO Device
 * @param dev SDIO Device
 * @note At least seven HCLK clock periods are needed between two write
 *       accesses to this register.
 */
void sdio_power_on(sdio_dev *dev) {
    dev->regs->POWER = (~SDIO_POWER_RESERVED & SDIO_POWER_ON);
}

/**
 * @brief Power off the SDIO Device
 * @param dev SDIO Device
 * @note At least seven HCLK clock periods are needed between two write
 *       accesses to this register.
 */
void sdio_power_off(sdio_dev *dev) {
    dev->regs->POWER = (~SDIO_POWER_RESERVED & SDIO_POWER_OFF);
}

/**
 * @brief Set the Clock Control Register
 * @param dev SDIO Device
 * @param val Value of Clock Control Register data
 * @note seven HCLK clock periods are needed between two write accesses
 */
void sdio_set_clkcr(sdio_dev *dev, uint32 val) {
    dev->regs->CLKCR = (~SDIO_CLKCR_RESERVED & val);
}

/**
 * @brief Configure the Command Path State Machine
 * @param dev SDIO Device
 * @param spc Register space to clear
 * @param val Value of Clock Control Register data to load into spc
 * @note seven HCLK clock periods are needed between two write accesses
 * @note This assumes you're on a LeafLabs-style board
 *       (CYCLES_PER_MICROSECOND == 72, APB2 at 72MHz, APB1 at 36MHz).
 */
void sdio_cfg_clkcr(sdio_dev *dev, uint32 spc, uint32 val) {
    spc = (~SDIO_CLKCR_RESERVED & spc);
    uint32 temp = dev->regs->CLKCR;
    temp &= ~spc;
    temp |= (spc & val);
    dev->regs->CLKCR = temp;
}

/**
 * @brief Configure GPIO bit modes for use as an SDIO port's pins
 * @note 8-bit data bus width is only allowed for UHS-I cards
 */
void sdio_cfg_gpio(void) {
    //These gpio devices and pins are constant for the F1 line
    gpio_set_mode(GPIOC,   11, GPIO_INPUT_FLOATING); //SDIO_D3
    gpio_set_mode(GPIOC,   10, GPIO_INPUT_FLOATING); //SDIO_D2
    gpio_set_mode(GPIOC,    9, GPIO_INPUT_FLOATING); //SDIO_D1
    timer_set_mode(TIMER8,  4,      TIMER_DISABLED);
    gpio_set_mode(GPIOC,    8, GPIO_INPUT_FLOATING); //SDIO_D0
    timer_set_mode(TIMER8,  3,      TIMER_DISABLED);
    gpio_set_mode(GPIOC,   12,   GPIO_AF_OUTPUT_PP); //SDIO_CK
    gpio_set_mode(GPIOD,    2,   GPIO_AF_OUTPUT_PP); //SDIO_CMD
}

/*
 * SDIO hardware functions
 */

/**
 * @brief Enable SDIO Data Transfer
 * @param dev SDIO Device
 */
void sdio_dt_enable(sdio_dev *dev) {
    bb_peri_set_bit(&dev->regs->DCTRL, SDIO_DCTRL_DTEN_BIT, 1);
}

/**
 * @brief Disable SDIO Data Transfer
 * @param dev SDIO Device
 */
void sdio_dt_disable(sdio_dev *dev) {
    bb_peri_set_bit(&dev->regs->DCTRL, SDIO_DCTRL_DTEN_BIT, 0);
}

/**
 * @brief Enable SDIO peripheral clock
 * @param dev SDIO Device
 */
void sdio_clock_enable(sdio_dev *dev) {
    bb_peri_set_bit(&dev->regs->CLKCR, SDIO_CLKCR_CLKEN_BIT, 1);
}

/**
 * @brief Disable SDIO peripheral clock
 * @param dev SDIO Device
 */
void sdio_clock_disable(sdio_dev *dev) {
    bb_peri_set_bit(&dev->regs->CLKCR, SDIO_CLKCR_CLKEN_BIT, 0);
}

/**
 * @brief Enable DMA requests
 * @param dev SDIO device
 */
void sdio_dma_enable(sdio_dev *dev) {
    bb_peri_set_bit(&dev->regs->DCTRL, SDIO_DCTRL_DMAEN_BIT, 1);
}

/**
 * @brief Disable DMA requests
 * @param dev SDIO device
 */
void sdio_dma_disable(sdio_dev *dev) {
    bb_peri_set_bit(&dev->regs->DCTRL, SDIO_DCTRL_DMAEN_BIT, 0);
}

/**
 * @brief Configure DMA for host to receive transfer
 * @param dev SDIO device
 * @param rx_buf pointer to 32-bit memory address
 * @param count Number of transfers to receive
 * @note DMA channel conflicts: TIM5_CH2 and TIM7_UP / DAC_Channel2
 */
void sdio_cfg_dma_rx(sdio_dev *dev, uint32 *dst, uint16 count) {
    dma_init(DMA2);
    dma_setup_transfer(DMA2, DMA_CH4, //constant for STM32F1 line
                       &dev->regs->FIFO,    DMA_SIZE_32BITS,
                       dst,                 DMA_SIZE_32BITS,
                       DMA_MINC_MODE | DMA_TRNS_CMPLT | DMA_TRNS_ERR);
    dma_set_num_transfers(DMA2, DMA_CH4, count);
  //dma_attach_interrupt(DMA2, DMA_CH4, sdio_irq_dma_rx);
    dma_enable(DMA2, DMA_CH4);
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
void sdio_send_command(sdio_dev *dev, uint32 cmd) {
    uint32 temp = dev->regs->CMD;
    temp &= SDIO_CMD_RESERVED;
    temp |= (~SDIO_CMD_RESERVED & cmd);
    dev->regs->CMD = temp;
}

/**
 * @brief Get last command that recieved a response
 * @param dev SDIO Device 
 */
uint32 sdio_get_command(sdio_dev *dev) {
    uint32 resp = dev->regs->RESPCMD;
    return (~SDIO_RESPCMD_RESERVED & resp);
}

/**
 * @brief Gets response from the SDIO Device
 * @param dev SDIO Device
 * @param buf Number of the response buffer
 * @retval Copy of the 32-bit response buffer
 */
uint32 sdio_get_resp(sdio_dev *dev, uint32 buf) {
    switch (buf) {
      case 1:
        return dev->regs->RESP1;
        break;
      case 2:
        return dev->regs->RESP2;
        break;
      case 3:
        return dev->regs->RESP3;
        break;
      case 4:
        return dev->regs->RESP4;
        break;
      default:
        return 0xFFFFFFFF; //chosen bc every status should be an error
    }
}

/*
 * SDIO status functions
 */

/**
 * @brief Detects if card is inserted
 */
uint32 sdio_card_detect(void) {
    gpio_set_mode(GPIOC, 11, GPIO_INPUT_PD); //SDIO_D3
    sdio_cfg_gpio();
    delay_us(1000);
    if (gpio_read_bit(GPIOC, 11)) {
        return 1;
    }
    return 0;
}

uint32 sdio_card_powered(sdio_dev *dev) {
    int i;
    for (i = 1; i <=5; i++) {
        if (dev->regs->POWER == SDIO_POWER_ON) {
            return 1;
        }
    }
    return 0;
}

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
    return bb_peri_get_bit(&dev->regs->STA, SDIO_STA_TXACT_BIT);
}

/**
 * @brief 
 * @param dev SDIO Device
 */
uint32 sdio_is_cmd_act(sdio_dev *dev) {
    return bb_peri_get_bit(&dev->regs->STA, SDIO_STA_CMDACT_BIT);
}

/*
 * SDIO data functions
 */

/**
 * @brief Configure the Data Control Register
 * @param dev SDIO Device
 * @param spc Register space to clear
 * @param val Value of Data Control Register data to load into spc
 */
void sdio_cfg_dcr(sdio_dev *dev, uint32 spc, uint32 val) {
    uint32 temp = dev->regs->DCTRL;
    temp &= ~spc;
    dev->regs->DCTRL |= (~SDIO_DCTRL_RESERVED & (spc & val));
}

/**
 * @brief Set the Data Control Register
 * @param dev SDIO Device
 * @param val Value of Data Control Register data to load
 */
void sdio_set_dcr(sdio_dev *dev, uint32 val) {
    dev->regs->DCTRL = (~SDIO_DCTRL_RESERVED & val);
}

/**
 * @brief Set data length value
 * @param dev SDIO Device 
 * @param length Length of data to transfer
 */
void sdio_set_data_length(sdio_dev *dev, uint32 length) {
    dev->regs->DLEN = (~SDIO_DLEN_RESERVED & length);
}

/**
 * @brief Set response timeout
 * @param dev SDIO Device 
 * @param timeout Timeout value for the data path state machine
 */
void sdio_set_data_timeout(sdio_dev *dev, uint32 timeout) {
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
 * @param dev SDIO Device
 * @retval Remaining number of words
 */
uint32 sdio_get_fifo_count(sdio_dev *dev) {
    return dev->regs->FIFOCNT;
}

/**
 * @brief  Read one data word from Rx FIFO
 * @param dev SDIO Device
 * @retval Data received
 */
uint32 sdio_read_data(sdio_dev *dev) {
    return dev->regs->FIFO;
}

/**
 * @brief  Write one data word to Tx FIFO
 * @param dev SDIO Device
 * @param  data 32-bit data word to write
 * @retval None
 */
void sdio_write_data(sdio_dev *dev, uint32 data) {
    dev->regs->FIFO = data;
}

/*
 * SDIO interrupt functions
 */

/**
 * @brief Checks whether the specified SDIO interrupt has occurred or not
 * @param dev SDIO Device
 * @param rupt Specifies the SDIO interrupt source to check
 * @retval Status of the interrupt, asserted: 1, deasserted: 0
 */
uint32 sdio_get_status(sdio_dev *dev, uint32 flag) { 
    if (dev->regs->STA & flag) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Clears the SDIO's pending interrupt flags
 * @param dev SDIO Device
 * @param flag Specifies the flag to clear
 */
void sdio_clear_interrupt(sdio_dev *dev, uint32 flag) {
    dev->regs->ICR = ~SDIO_ICR_RESERVED & flag;
}

/**
 * @brief Add interrupt flag to generate an interrupt request
 * @param dev SDIO Device
 * @param mask Interrupt sources to enable
 */
void sdio_add_interrupt(sdio_dev *dev, uint32 mask) {
    dev->regs->MASK |= mask;
}

/**
 * @brief Writes interrupt mask to generate an interrupt request
 * @param dev SDIO Device
 * @param mask Interrupt sources to enable
 */
void sdio_set_interrupt(sdio_dev *dev, uint32 mask) {
    dev->regs->MASK = ~SDIO_MASK_RESERVED & mask;
}