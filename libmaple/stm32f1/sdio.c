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
 * @file libmaple/stm32f1/sdio.c
 * @author Brian E Tovar <betovar@leaflabs.com>
 * @brief 
 */

#include <libmaple/sdio.h>

//These gpio values are constant for STM32F103xE chips
#define BOARD_SDIO_D0_PIN       15
#define BOARD_SDIO_D1_PIN       16
#define BOARD_SDIO_D2_PIN       17
#define BOARD_SDIO_D3_PIN       18
#define BOARD_SDIO_CLK_PIN      19
#define BOARD_SDIO_CMD_PIN      27
#define BOARD_SDIO_PWR_PIN      26
#define BOARD_SDIO_CD_PIN       14

#define BOARD_SDIO_D0_DEV      GPIOC
#define BOARD_SDIO_D0_BIT      8
#define BOARD_SDIO_D1_DEV      GPIOC
#define BOARD_SDIO_D1_BIT      9
#define BOARD_SDIO_D2_DEV      GPIOC
#define BOARD_SDIO_D2_BIT      10
#define BOARD_SDIO_D3_DEV      GPIOC
#define BOARD_SDIO_D3_BIT      11
#define BOARD_SDIO_CLK_DEV     GPIOC
#define BOARD_SDIO_CLK_BIT     12
#define BOARD_SDIO_CMD_DEV     GPIOD
#define BOARD_SDIO_CMD_BIT     2
#define BOARD_SDIO_PWR_DEV     GPIOB
#define BOARD_SDIO_PWR_BIT     9
#define BOARD_SDIO_CD_DEV      GPIOC
#define BOARD_SDIO_CD_BIT      7

/*
 * SDIO device
 */

#if defined(STM32_HIGH_DENSITY) || defined(STM32_XL_DENSITY)
static sdio_dev sdio = {
    .regs     = SDIO_BASE,
    .clk_id   = RCC_SDIO,
    .irq_num  = NVIC_SDIO,
};

sdio_dev *SDIO = &sdio;
#endif

/**
 * @brief Configure GPIO bit modes for use as an SDIO port's pins
 * @note 8-bit data bus width is only allowed for UHS-I cards
 */
void sdio_cfg_gpio(void) {
    gpio_set_mode(BOARD_SDIO_D0_DEV,
                  BOARD_SDIO_D0_BIT,
                  GPIO_INPUT_FLOATING);
    gpio_set_mode(BOARD_SDIO_D1_DEV,
                  BOARD_SDIO_D1_BIT,
                  GPIO_INPUT_FLOATING);
    gpio_set_mode(BOARD_SDIO_D2_DEV,
                  BOARD_SDIO_D2_BIT,
                  GPIO_INPUT_FLOATING);
    gpio_set_mode(BOARD_SDIO_D3_DEV,
                  BOARD_SDIO_D3_BIT,
                  GPIO_INPUT_FLOATING);
    gpio_set_mode(BOARD_SDIO_CLK_DEV,
                  BOARD_SDIO_CLK_BIT,
                  GPIO_AF_OUTPUT_PP);
    gpio_set_mode(BOARD_SDIO_CMD_DEV,
                  BOARD_SDIO_CMD_BIT,
                  GPIO_AF_OUTPUT_PP);
    gpio_set_mode(BOARD_SDIO_PWR_DEV,
                  BOARD_SDIO_PWR_BIT,
                  GPIO_OUTPUT_PP);
    gpio_set_mode(BOARD_SDIO_CD_DEV,
                  BOARD_SDIO_CD_BIT,
                  GPIO_INPUT_PD);
    timer_set_mode(TIMER8, 3, TIMER_DISABLED);
    timer_set_mode(TIMER8, 4, TIMER_DISABLED);
}

/**
 * @brief Detects if card is inserted
 */
uint32 sdio_card_detect(void) {
    gpio_set_mode(BOARD_SDIO_D3_DEV, BOARD_SDIO_D3_BIT, GPIO_INPUT_PD);
    sdio_cfg_gpio();
    delay_us(1000);
    if (gpio_read_bit(BOARD_SDIO_D3_DEV, BOARD_SDIO_D3_BIT)) {
        return 1;
    } else {
        return 0;        
    }
}