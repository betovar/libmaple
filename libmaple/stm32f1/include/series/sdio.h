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
#include <libmaple/dma.h>
#include <libmaple/rcc.h>
#include <libmaple/nvic.h>
#include <libmaple/util.h>
#include <libmaple/bitband.h>
#include <libmaple/gpio.h>
#include <libmaple/timer.h>
#include <libmaple/delay.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Register map base pointers
 */

struct sdio_reg_map;

#define SDIO_BASE                   ((struct sdio_reg_map*)0x40018000)

/*
 * Device pointers
 */

struct sdio_dev;

#if defined(STM32_HIGH_DENSITY) || defined(STM32_XL_DENSITY)
extern struct sdio_dev *SDIO;
#endif

/*
 * Routines
 */

/* SDIO DMA device and channel (constant for STM32F1 line) */
#define SDIO_DMA_DEVICE  DMA2 //FIXME: fix for multi- family support
#define SDIO_DMA_CHANNEL DMA_CH6

void sdio_cfg_gpio(void);
uint32 sdio_card_detect(void);

#ifdef __cplusplus
}
#endif

#endif