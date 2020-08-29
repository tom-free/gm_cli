/*******************************************************************************
** 文件名称：hal_gpio_bitband.h
** 文件作用：GPIO位带操作
** 编写作者：Tom Free 付瑞彪
** 编写时间：2020-06-21
** 文件备注：
**			 
** 更新记录：
**          2020-06-21 -> 创建文件                             <Tom Free 付瑞彪>
**
**              Copyright (c) 2018-2020 付瑞彪 All Rights Reserved
**
**       1 Tab == 4 Spaces     UTF-8     ANSI C Language(C99)
*******************************************************************************/
#ifndef __HAL_GPIO_BIT_BAND_H__
#define __HAL_GPIO_BIT_BAND_H__

#include "stm32f10x.h"

/* IO口操作宏定义 */
#define BIT_BAND(addr, bitnum)  ((addr & 0xF0000000) + 0x2000000 + ((addr & 0xFFFFF) << 5) + (bitnum << 2))
#define MEM_ADDR(addr)          (*((volatile unsigned long *)(addr)))
#define BIT_ADDR(addr, bitnum)  MEM_ADDR(BIT_BAND(addr, bitnum)) 

/* IO口ODR寄存器地址映射 */
#define GPIOA_ODR_ADDR      (GPIOA_BASE + 12)   /* 0x4001080C */
#define GPIOB_ODR_ADDR      (GPIOB_BASE + 12)   /* 0x40010C0C */
#define GPIOC_ODR_ADDR      (GPIOC_BASE + 12)   /* 0x4001100C */
#define GPIOD_ODR_ADDR      (GPIOD_BASE + 12)   /* 0x4001140C */
#define GPIOE_ODR_ADDR      (GPIOE_BASE + 12)   /* 0x4001180C */
#define GPIOF_ODR_ADDR      (GPIOF_BASE + 12)   /* 0x40011A0C */   
#define GPIOG_ODR_ADDR      (GPIOG_BASE + 12)   /* 0x40011E0C */   

/* IO口IDR寄存器地址映射 */
#define GPIOA_IDR_ADDR      (GPIOA_BASE + 8)    /* 0x40010808 */
#define GPIOB_IDR_ADDR      (GPIOB_BASE + 8)    /* 0x40010C08 */
#define GPIOC_IDR_ADDR      (GPIOC_BASE + 8)    /* 0x40011008 */
#define GPIOD_IDR_ADDR      (GPIOD_BASE + 8)    /* 0x40011408 */
#define GPIOE_IDR_ADDR      (GPIOE_BASE + 8)    /* 0x40011808 */
#define GPIOF_IDR_ADDR      (GPIOF_BASE + 8)    /* 0x40011A08 */
#define GPIOG_IDR_ADDR      (GPIOG_BASE + 8)    /* 0x40011E08 */

#define STR_CONNENT(a, b, c)    a ## b ## c

/* IO口输入输出操作,只对单一的IO口 */
/* 确保portx的值为A-G */
/* 确保n的值小于16 */
#define GPIO_OUT(portx, n)  BIT_ADDR(STR_CONNENT(GPIO, portx, _ODR_ADDR) ,n) /* 输出 */
#define GPIO_IN(portx, n)   BIT_ADDR(STR_CONNENT(GPIO, portx, _IDR_ADDR), n) /* 输入 */

#endif  /* __HAL_GPIO_BIT_BAND_H__ */
