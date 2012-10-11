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
void sdio_init(void) {
    rcc_clk_enable(SDIO->clk_id);
    rcc_reset_dev(SDIO->clk_id);
  //nvic_irq_enable(SDIO->irq_num);
}

/**
 * @brief Reset an SDIO Device
 */
void sdio_reset(void) {
    rcc_reset_dev(SDIO->clk_id);
    SDIO->regs->POWER  = 0x00000000;
    SDIO->regs->CLKCR  = 0x00000000;
    SDIO->regs->ARG    = 0x00000000;
    SDIO->regs->CMD    = 0x00000000;
    SDIO->regs->DTIMER = 0x00000000;
    SDIO->regs->DLEN   = 0x00000000;
    SDIO->regs->DCTRL  = 0x00000000;
    SDIO->regs->ICR    = 0x00C007FF;
    SDIO->regs->MASK   = 0x00000000;
  //nvic_irq_disable(SDIO->irq_num);
}

/**
 * @brief Set the Clock Control Register
 * @param val Value of Clock Control Register data
 * @note seven HCLK clock periods are needed between two write accesses
 */
void sdio_set_clkcr(uint32 val) {
    SDIO->regs->CLKCR = (~SDIO_CLKCR_RESERVED & val);
}

/**
 * @brief Configure the Command Path State Machine
 * @param spc Register space to clear
 * @param val Value of Clock Control Register data to load into spc
 * @note seven HCLK clock periods are needed between two write accesses
 * @note This assumes you're on a LeafLabs-style board
 *       (CYCLES_PER_MICROSECOND == 72, APB2 at 72MHz, APB1 at 36MHz).
 */
void sdio_cfg_clkcr(uint32 spc, uint32 val) {
    spc = ~SDIO_CLKCR_RESERVED & spc;
    uint32 temp = SDIO->regs->CLKCR;
    temp &= ~spc;
    temp |= (spc & val);
    SDIO->regs->CLKCR = temp;
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
 * SDIO DMA functions
 */

/**
 * @brief Configure DMA for host to receive data
 * @param rx_buf pointer to 32-bit memory address
 * @param count Number of transfers to receive
 * @note DMA channel conflicts: TIM5_CH2 and TIM7_UP / DAC_Channel2
 */
void sdio_cfg_dma_rx(uint32 *dst, uint16 count) {
    dma_init(SDIO_DMA_DEVICE);
    dma_setup_transfer(SDIO_DMA_DEVICE,   SDIO_DMA_CHANNEL,
                       &SDIO->regs->FIFO, DMA_SIZE_32BITS,
                       dst,               DMA_SIZE_32BITS,
                       DMA_MINC_MODE | DMA_TRNS_CMPLT | DMA_TRNS_ERR);
    dma_set_num_transfers(SDIO_DMA_DEVICE, SDIO_DMA_CHANNEL, count);
    dma_attach_interrupt(SDIO_DMA_DEVICE, SDIO_DMA_CHANNEL, sdio_dma_rx_irq);
    dma_enable(SDIO_DMA_DEVICE, SDIO_DMA_CHANNEL);
}

/**
 * @brief Configure DMA for host to trasmit data
 * @param rx_buf pointer to 32-bit memory address
 * @param count Number of transfers to receive
 * @note DMA channel conflicts: TIM5_CH2 and TIM7_UP / DAC_Channel2
 */
void sdio_cfg_dma_tx(uint32 *src, uint16 count) {
    /*
    4.  Configure the DMA2 as follows:
    a)  Enable DMA2 controller and clear any pending interrupts
    b)  Program the DMA2_Channel4 source address register with the memory 
        location’s base address and DMA2_Channel4 destination address register 
        with the SDIO_FIFO register address
    c)  Program DMA2_Channel4 control register (memory increment, not 
        peripheral increment, peripheral and source width is word size)
    d)  Enable DMA2_Channel4
    */
    dma_init(SDIO_DMA_DEVICE);
    dma_setup_transfer(SDIO_DMA_DEVICE,   SDIO_DMA_CHANNEL, 
                       &SDIO->regs->FIFO, DMA_SIZE_32BITS,
                       src,               DMA_SIZE_32BITS,
                       DMA_MINC_MODE | DMA_TRNS_CMPLT | DMA_TRNS_ERR);
    dma_set_num_transfers(SDIO_DMA_DEVICE, SDIO_DMA_CHANNEL, count);
    dma_attach_interrupt(SDIO_DMA_DEVICE, SDIO_DMA_CHANNEL, sdio_dma_tx_irq);
    dma_enable(SDIO_DMA_DEVICE, SDIO_DMA_CHANNEL);
}

/**
 * @brief  
 */
void sdio_dma_rx_irq(void) {
    if (sdio_check_status(SDIO_STA_DTIMEOUT)) {}
    if (sdio_check_status(SDIO_STA_STBITERR)) {}
    if (sdio_check_status(SDIO_STA_RXFIFOE)) {}
    if (sdio_check_status(SDIO_STA_RXFIFOF)) {}
    if (sdio_check_status(SDIO_STA_RXFIFOHF)) {}
    if (sdio_check_status(SDIO_STA_RXOVERR)) {}
    if (sdio_check_status(SDIO_STA_RXDAVL)) {}
    if (sdio_check_status(SDIO_STA_DATAEND)) {}
    if (sdio_check_status(SDIO_STA_DCRCFAIL)) {}
    if (sdio_check_status(SDIO_STA_DBCKEND)) {
        sdio_dma_disable();
    }
}

/**
 * @brief  
 */
void sdio_dma_tx_irq(void) {
    dma_irq_cause cause;
    cause = dma_get_irq_cause(SDIO_DMA_DEVICE, SDIO_DMA_CHANNEL);
    switch (cause) {
      case DMA_TRANSFER_HALF_COMPLETE:
        break;
      case DMA_TRANSFER_COMPLETE:
        if (sdio_check_status(SDIO_STA_DBCKEND)) {
            sdio_dma_disable();
        } else {

        }
        break;
      default:
      break;
    }
}

/*
 * SDIO command functions
 */

/**
 * @brief Load argument into SDIO Argument Register
 * @param arg Argument Data
 */
void sdio_load_arg(uint32 arg) {
    SDIO->regs->ARG = arg;
}

/**
 * @brief Send command to external card
 * @param cmd SDIO Command to send 
 */
void sdio_send_command(uint32 cmd) {
    uint32 temp = SDIO->regs->CMD;
    temp &= SDIO_CMD_RESERVED;
    temp |= (~SDIO_CMD_RESERVED & cmd);
    SDIO->regs->CMD = temp;
}

/**
 * @brief Get last command that recieved a response 
 */
uint32 sdio_get_command(void) {
    uint32 resp = SDIO->regs->RESPCMD;
    return resp & SDIO_RESPCMD_RESPCMD;
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
    } else {
        return 0;        
    }
}

uint32 sdio_card_powered(void) {
    int i;
    for (i = 1; i <=5; i++) {
        if (SDIO->regs->POWER == SDIO_POWER_ON) {
            return 1;
        }
    }
    return 0;
}

/*
 * SDIO data functions
 */

/**
 * @brief Configure the Data Control Register
 * @param spc Register space to clear
 * @param val Value of Data Control Register data to load into spc
 */
void sdio_cfg_dcr(uint32 spc, uint32 val) {
    uint32 temp = SDIO->regs->DCTRL;
    temp &= ~spc;
    SDIO->regs->DCTRL |= (~SDIO_DCTRL_RESERVED & (spc & val));
}

/**
 * @brief Set the Data Control Register
 * @param val Value of Data Control Register data to load
 */
void sdio_set_dcr(uint32 val) {
    SDIO->regs->DCTRL = (~SDIO_DCTRL_RESERVED & val);
}

/**
 * @brief Set data length value
 * @param length Length of data to transfer
 */
void sdio_set_data_length(uint32 length) {
    SDIO->regs->DLEN = (~SDIO_DLEN_RESERVED & length);
}

/**
 * @brief Set response timeout
 * @param timeout Timeout value for the data path state machine
 */
void sdio_set_data_timeout(uint32 timeout) {
    SDIO->regs->DTIMER = timeout;
}

/**
 * @brief  Part of the data path state machine, this (read-only)register
 *         loads the value from the data length register and decrements
 *         until 0.
 * @retval Number of remaining data bytes to be transferred 
 */
uint32 sdio_get_data_count(void) {
    return SDIO->regs->DCOUNT;
}

/**
 * @brief  Returns the number of words left to be written to or read from FIFO
 * @retval Remaining number of words
 */
uint32 sdio_get_fifo_count(void) {
    return SDIO->regs->FIFOCNT;
}

/**
 * @brief  Read one data word from Rx FIFO
 * @retval Data received
 */
uint32 sdio_read_data(void) {
    return SDIO->regs->FIFO;
}

/**
 * @brief  Write one data word to Tx FIFO
 * @param  data 32-bit data word to write
 * @retval None
 */
void sdio_write_data(uint32 data) {
    SDIO->regs->FIFO = data;
}

/**
 * @brief IRQ for SDIO peripheral
 */
void __irq_sdio(void) {
}