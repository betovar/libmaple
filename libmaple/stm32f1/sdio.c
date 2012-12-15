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
    //These gpio devices and pins are constant for the F1 line
    gpio_set_mode(GPIOD,    2,   GPIO_AF_OUTPUT_PP); //SDIO_CMD
    gpio_set_mode(GPIOC,   12,   GPIO_AF_OUTPUT_PP); //SDIO_CK
    gpio_set_mode(GPIOC,   11, GPIO_INPUT_FLOATING); //SDIO_D3
    gpio_set_mode(GPIOC,   10, GPIO_INPUT_FLOATING); //SDIO_D2
    gpio_set_mode(GPIOC,    9, GPIO_INPUT_FLOATING); //SDIO_D1
    gpio_set_mode(GPIOC,    8, GPIO_INPUT_FLOATING); //SDIO_D0
    timer_set_mode(TIMER8,  3,      TIMER_DISABLED);
    timer_set_mode(TIMER8,  4,      TIMER_DISABLED);
}

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