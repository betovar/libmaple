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

#include <libmaple/sdio.h>

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

/*
 * SDIO other functions
 */

/**
 * @brief Enable SDIO Data Transfer
 */
void sdio_dt_enable(void) {
    bb_peri_set_bit(&SDIO->regs->DCTRL, SDIO_DCTRL_DTEN_BIT, 1);
}

/**
 * @brief Disable SDIO Data Transfer
 */
void sdio_dt_disable(void) {
    bb_peri_set_bit(&SDIO->regs->DCTRL, SDIO_DCTRL_DTEN_BIT, 0);
}

/**
 * @brief Enable SDIO peripheral clock
 */
void sdio_clock_enable(void) {
    bb_peri_set_bit(&SDIO->regs->CLKCR, SDIO_CLKCR_CLKEN_BIT, 1);
}

/**
 * @brief Disable SDIO peripheral clock
 */
void sdio_clock_disable(void) {
    bb_peri_set_bit(&SDIO->regs->CLKCR, SDIO_CLKCR_CLKEN_BIT, 0);
}

/**
 * @brief Enable DMA requests
 */
void sdio_dma_enable(void) {
    bb_peri_set_bit(&SDIO->regs->DCTRL, SDIO_DCTRL_DMAEN_BIT, 1);
}

/**
 * @brief Disable DMA requests
 */
void sdio_dma_disable(void) {
    bb_peri_set_bit(&SDIO->regs->DCTRL, SDIO_DCTRL_DMAEN_BIT, 0);
}

/**
 * @brief Gets response buffer 1 from the SDIO Device
 * @retval Copy of the 32-bit response buffer
 */
uint32 sdio_get_resp1() {
    return SDIO->regs->RESP1;
}

/**
 * @brief Gets response buffer 2 from the SDIO Device
 * @retval Copy of the 32-bit response buffer
 */
uint32 sdio_get_resp2() {
    return SDIO->regs->RESP2;
}

/**
 * @brief Gets response buffer 3 from the SDIO Device
 * @retval Copy of the 32-bit response buffer
 */
uint32 sdio_get_resp3() {
    return SDIO->regs->RESP3;
}

/**
 * @brief Gets response buffer 4 from the SDIO Device
 * @retval Copy of the 32-bit response buffer
 */
uint32 sdio_get_resp4() {
    return SDIO->regs->RESP4;
}

/**
 * @brief Is the command active?
 */
uint32 sdio_is_cmd_active(void) {
    return SDIO->regs->STA & SDIO_STA_CMDACT;
}

/**
 * @brief Is data transfer active?
 */
uint32 sdio_is_dt_active(void) {
    return SDIO->regs->STA & (SDIO_STA_RXACT | SDIO_STA_TXACT);
}

/**
 * @brief Is data (available) in FIFO?
 */
uint32 sdio_is_data_available(void) {
    return SDIO->regs->STA & (SDIO_STA_RXDAVL | SDIO_STA_TXDAVL);
}

/**
 * @brief Checks whether the specified SDIO interrupt has occurred or not
 * @param mask Specifies the SDIO interrupt source(s) to check
 * @retval Status of the masked interrupt(s)
 */
uint32 sdio_check_status(uint32 mask) {
    return SDIO->regs->STA & mask;
}

/**
 * @brief Clears the SDIO's pending interrupt flags
 * @param flag Specifies the interrupt to clear
 */
void sdio_clear_interrupt(uint32 flag) {
    SDIO->regs->ICR = ~SDIO_ICR_RESERVED & flag;
}

/**
 * @brief Add interrupt flag to generate an interrupt request
 * @param mask Interrupt sources to enable
 */
void sdio_add_interrupt(uint32 mask) { //FIXME
    SDIO->regs->MASK |= ~SDIO_MASK_RESERVED & mask;
}

/**
 * @brief Writes to interrupt mask register to generate an interrupt request
 * @param mask Interrupt sources to enable
 */
void sdio_enable_interrupt(uint32 mask) {
    SDIO->regs->MASK = ~SDIO_MASK_RESERVED & mask;
}

/**
 * @brief IRQ for SDIO peripheral
 */
void __irq_sdio(void) {
}