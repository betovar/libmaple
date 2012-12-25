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
 * @file libmaple/stm32f1/include/series/sdio.h
 * @author Brian E Tovar <betovar@leaflabs.com>
 * @brief 
 */

#ifndef _LIBMAPLE_STM32F1_SDIO_H_
#define _LIBMAPLE_STM32F1_SDIO_H_

#include <libmaple/libmaple_types.h>
#include <libmaple/sdio.h>

/* SDIO DMA device and channel (constant for STM32F1 line) */
#define SDIO_DMA_DEVICE  DMA2 //FIXME: fix for multi- family support
#define SDIO_DMA_CHANNEL DMA_CH6

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

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Register map base pointers
 */

struct sdio_reg_map;

#define SDIO_BASE ((struct sdio_reg_map*)0x40018000)

/*
 * Device pointers
 */

struct sdio_dev;

#if defined(STM32_HIGH_DENSITY) || defined(STM32_XL_DENSITY)
extern struct sdio_dev *SDIO;
#endif

void sdio_cfg_gpio(void);
void sdio_power_on(void);
void sdio_power_off(void);
uint32 sdio_card_powered(void);
uint32 sdio_card_detect(void);

#ifdef __cplusplus
}
#endif

#endif