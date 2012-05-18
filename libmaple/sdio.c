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
    //nvic_irq_enable(dev->irg_num);
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
 */
void sdio_cfg_clkcr(sdio_dev *dev, uint32 spc, uint32 val) {
    spc = (~SDIO_CLKCR_RESERVED & spc);
    uint32 temp = dev->regs->CLKCR;
    temp &= ~spc;
    temp |= (spc & val);
    dev->regs->CLKCR = temp;
}

/**
 * @brief Set Clock Divisor in the Clock Control Register
 * @param dev SDIO
 * @param div clock divider factor to set the sdio_ck frequency
 * @note This assumes you're on a LeafLabs-style board
 *       (CYCLES_PER_MICROSECOND == 72, APB2 at 72MHz, APB1 at 36MHz).
 */
void sdio_cfg_clock(sdio_dev *dev, uint8 div) {
    /* CLKDIV: Set Clock Divider SDIOCLK/[CLKDIV+2]. */
    uint32 temp =dev->regs->CLKCR;
    temp &= ~SDIO_CLKCR_CLKDIV;
    temp |= (uint32)div;
    dev->regs->CLKCR = temp;
}

/**
 * @brief Configure GPIO bit modes for use as an SDIO port's pins
 * @param width Bus width to configure pins for use as an SDIO card
 * @note 8-bit data bus width is only allowed for UHS-I cards
 */
void sdio_cfg_gpio(uint8 width) {
    //timer_set_mode();
    //These gpio pins are constant for the F1 line
    switch (width) {
    case SDIO_GPIO_CARD_DETECT:
        gpio_set_mode(GPIOC, 11,       GPIO_INPUT_PD); //SDIO_D3
        //delay_us(1000);
        //gpio_read_bit(GPIOC, 11);
    case SDIO_GPIO_INIT:
        gpio_set_mode(GPIOC, 11, GPIO_INPUT_FLOATING); //SDIO_D3
        gpio_set_mode(GPIOC, 10, GPIO_INPUT_FLOATING); //SDIO_D2
        gpio_set_mode(GPIOC, 9,  GPIO_INPUT_FLOATING); //SDIO_D1
        timer_set_mode(TIMER8, 4,     TIMER_DISABLED);
        gpio_set_mode(GPIOC, 8,  GPIO_INPUT_FLOATING); //SDIO_D0
        timer_set_mode(TIMER8, 3,     TIMER_DISABLED);
        gpio_set_mode(GPIOC, 12,   GPIO_AF_OUTPUT_PP); //SDIO_CK
    case SDIO_GPIO_CMD_OUTPUT:
        gpio_set_mode(GPIOD, 2,    GPIO_AF_OUTPUT_PP); //SDIO_CMD
        break;
    case SDIO_GPIO_CMD_INPUT:
        gpio_set_mode(GPIOD, 2,  GPIO_INPUT_FLOATING); //SDIO_CMD
        break;
    case SDIO_GPIO_4B_DATA_INPUT:
        gpio_set_mode(GPIOC, 11, GPIO_INPUT_FLOATING); //SDIO_D3
        gpio_set_mode(GPIOC, 10, GPIO_INPUT_FLOATING); //SDIO_D2
        gpio_set_mode(GPIOC, 9,  GPIO_INPUT_FLOATING); //SDIO_D1
    case SDIO_GPIO_1B_DATA_INPUT:
        gpio_set_mode(GPIOC, 8,  GPIO_INPUT_FLOATING); //SDIO_D0
        break;
    case SDIO_GPIO_4B_DATA_OUTPUT:
        gpio_set_mode(GPIOC, 11,   GPIO_AF_OUTPUT_PP); //SDIO_D3
        gpio_set_mode(GPIOC, 10,   GPIO_AF_OUTPUT_PP); //SDIO_D2
        gpio_set_mode(GPIOC, 9,    GPIO_AF_OUTPUT_PP); //SDIO_D1
    case SDIO_GPIO_1B_DATA_OUTPUT:
        gpio_set_mode(GPIOC, 8,    GPIO_AF_OUTPUT_PP); //SDIO_D0
        break;
    default: // Error catch
        ASSERT(0);
    } //end of switch case
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
 * @brief Configure DMA
 * @param dev SDIO device
 */
void sdio_cfg_dma(sdio_dev *dev) {
    //other things go here
    bb_peri_set_bit(&dev->regs->DCTRL, SDIO_DCTRL_DMAEN_BIT, 1);
/**
  * @brief  Configures the DMA2 Channel4 for SDIO Tx request.
  * @param  BufferSRC: pointer to the source buffer
  * @param  BufferSize: buffer size
  * @retval None
void SD_LowLevel_DMA_TxConfig(uint32_t *BufferSRC, uint32_t BufferSize)
{
  DMA_InitTypeDef SDDMA_InitStructure;

  DMA_ClearFlag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_FEIF | 
  SD_SDIO_DMA_FLAG_DMEIF | SD_SDIO_DMA_FLAG_TEIF | 
  SD_SDIO_DMA_FLAG_HTIF | SD_SDIO_DMA_FLAG_TCIF);

  // DMA2 Stream3  or Stream6 disable 
  DMA_Cmd(SD_SDIO_DMA_STREAM, DISABLE);

  // DMA2 Stream3  or Stream6 Config 
  DMA_DeInit(SD_SDIO_DMA_STREAM);

  SDDMA_InitStructure.DMA_Channel = SD_SDIO_DMA_CHANNEL;
  SDDMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)SDIO_FIFO_ADDRESS;
  SDDMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BufferSRC;
  SDDMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  SDDMA_InitStructure.DMA_BufferSize = 0;
  SDDMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  SDDMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  SDDMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
  SDDMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  SDDMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  SDDMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  SDDMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
  SDDMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  SDDMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
  SDDMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;
  DMA_Init(SD_SDIO_DMA_STREAM, &SDDMA_InitStructure);
  DMA_ITConfig(SD_SDIO_DMA_STREAM, DMA_IT_TC, ENABLE);
  DMA_FlowControllerConfig(SD_SDIO_DMA_STREAM, DMA_FlowCtrl_Peripheral);

  // DMA2 Stream3  or Stream6 enable 
  DMA_Cmd(SD_SDIO_DMA_STREAM, ENABLE);


  * @brief  Configures the DMA2 Channel4 for SDIO Rx request.
  * @param  BufferDST: pointer to the destination buffer
  * @param  BufferSize: buffer size
  * @retval None

void SD_LowLevel_DMA_RxConfig(uint32_t *BufferDST, uint32_t BufferSize)
{
  DMA_InitTypeDef SDDMA_InitStructure;

  DMA_ClearFlag(SD_SDIO_DMA_STREAM, SD_SDIO_DMA_FLAG_FEIF | 
    SD_SDIO_DMA_FLAG_DMEIF | SD_SDIO_DMA_FLAG_TEIF | SD_SDIO_DMA_FLAG_HTIF | 
    SD_SDIO_DMA_FLAG_TCIF);

  // DMA2 Stream3  or Stream6 disable 
  DMA_Cmd(SD_SDIO_DMA_STREAM, DISABLE);

  // DMA2 Stream3 or Stream6 Config 
  DMA_DeInit(SD_SDIO_DMA_STREAM);

  SDDMA_InitStructure.DMA_Channel = SD_SDIO_DMA_CHANNEL;
  SDDMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)SDIO_FIFO_ADDRESS;
  SDDMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BufferDST;
  SDDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  SDDMA_InitStructure.DMA_BufferSize = 0;
  SDDMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  SDDMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  SDDMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
  SDDMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  SDDMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  SDDMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  SDDMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
  SDDMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  SDDMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
  SDDMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;
  DMA_Init(SD_SDIO_DMA_STREAM, &SDDMA_InitStructure);
  DMA_ITConfig(SD_SDIO_DMA_STREAM, DMA_IT_TC, ENABLE);
  DMA_FlowControllerConfig(SD_SDIO_DMA_STREAM, DMA_FlowCtrl_Peripheral);

  // DMA2 Stream3 or Stream6 enable
  DMA_Cmd(SD_SDIO_DMA_STREAM, ENABLE);
*/
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
void sdio_send_cmd(sdio_dev *dev, uint32 cmd) {
    dev->regs->CMD = (~SDIO_CMD_RESERVED & cmd);
}

/**
 * @brief Get last command that recieved a response
 * @param dev SDIO Device 
 */
uint32 sdio_get_cmd(sdio_dev *dev) {
    uint32 resp = dev->regs->RESPCMD;
    return (~SDIO_RESPCMD_RESERVED & resp);
}

/**
 * @brief Get short response
 * @param dev SDIO Device
 * @retval The 32-bit short response
 */
void sdio_get_resp_short(sdio_dev *dev, uint32 *buf) {
    buf[0] = dev->regs->RESP1;
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

/*
 * SDIO status functions
 */

/**
 * @brief Detects if card is inserted
 */
uint32 sdio_card_detect(void) {
    sdio_cfg_gpio(SDIO_GPIO_CARD_DETECT);
    delay_us(1000);
    if (gpio_read_bit(GPIOC, 11)) {
        return 1;
    }
    return 0;
}

uint32 sdio_is_power(sdio_dev *dev) {
    return dev->regs->POWER;
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
 * @brief Clears the SDIO's pending flags
 * @param dev SDIO Device
 * @param flag Specifies the flag to clear
 */
void sdio_clear_interrupt(sdio_dev *dev, uint32 flag) {
    dev->regs->ICR = ~SDIO_ICR_RESERVED & flag;
}

/**
 * @brief Determines which interrupt flags generate an interrupt request
 * @param dev SDIO Device
 * @param mask Interrupt sources to enable or disable
 * @param state The new state of the specified SDIO interrupts
 */
void sdio_cfg_interrupt(sdio_dev *dev, uint32 mask, uint8 state) {
    switch (state) {
    case SDIO_MASK_STATE_DISABLE:
    /* Disable the SDIO interrupts */
        dev->regs->MASK &= ~mask;
        break;
    case SDIO_MASK_STATE_ENABLE:
    /* Enable the SDIO interrupts */
        dev->regs->MASK |= mask;
        break;
    case SDIO_MASK_STATE_WRITE:
    /* Configure entire interrupt mask */
        dev->regs->MASK = mask;
        break;
    default:
        break;
    } 
}