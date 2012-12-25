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

//#include <libmaple/sdio.h>
#include <series/sdio.h>
#include <libmaple/timer.h>
#include <libmaple/gpio.h>
#include <libmaple/dma.h>
//#include <board/board.h>

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

void sdio_cfg_gpio(void) {
#ifdef BOARD_SDIO_PWR_PIN
    gpio_write_bit(BOARD_SDIO_PWR_DEV, BOARD_SDIO_PWR_BIT, 0);
    gpio_set_mode(BOARD_SDIO_PWR_DEV,
                  BOARD_SDIO_PWR_BIT,
                  GPIO_OUTPUT_PP);
#endif
#ifndef BOARD_SDIO_CD_PIN
    gpio_set_mode(BOARD_SDIO_D3_DEV,
                  BOARD_SDIO_D3_BIT,
                  GPIO_INPUT_PD);
#else
    gpio_set_mode(BOARD_SDIO_CD_DEV,
                  BOARD_SDIO_CD_BIT,
                  GPIO_INPUT_PD);
#endif
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
    timer_set_mode(TIMER8, 3, TIMER_DISABLED);
    timer_set_mode(TIMER8, 4, TIMER_DISABLED);
}

/**
 * @brief Power on the SDIO Device
 * @note At least seven HCLK clock periods are needed between two write
 *       accesses to this register.
 */
void sdio_power_on(void) {
    SDIO->regs->POWER = ~SDIO_POWER_RESERVED & SDIO_POWER_ON;
#ifdef BOARD_SDIO_PWR_PIN
    gpio_write_bit(BOARD_SDIO_PWR_DEV, BOARD_SDIO_PWR_BIT, 1);
#endif
}

/**
 * @brief Power off the SDIO Device
 * @note At least seven HCLK clock periods are needed between two write
 *       accesses to this register.
 */
void sdio_power_off(void) {
    SDIO->regs->POWER = ~SDIO_POWER_RESERVED & SDIO_POWER_OFF;
#ifdef BOARD_SDIO_PWR_PIN
    gpio_write_bit(BOARD_SDIO_PWR_DEV, BOARD_SDIO_PWR_BIT, 0);
#endif
}

/**
 * @brief Detects if card is powered
 */
uint32 sdio_card_powered(void) {
    if (SDIO->regs->POWER == SDIO_POWER_ON) {
        //FIXME: add gpio check
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Detects if card is inserted
 */
uint32 sdio_card_detect(void) {
#ifdef BOARD_SDIO_CD_PIN // FIXME: cleverer way to do this?
    return gpio_read_bit(BOARD_SDIO_CD_DEV, BOARD_SDIO_CD_BIT);
#else
    return gpio_read_bit(BOARD_SDIO_D3_DEV, BOARD_SDIO_D3_BIT);
#endif
}