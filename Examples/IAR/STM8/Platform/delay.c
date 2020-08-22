/*******************************************************************************
** 文件名称：main.c
** 文件作用：延时函数文件
** 编写作者：Tom Free 付瑞彪
** 编写时间：2018-10-31
** 文件备注：
**
**
** 更新记录：
**          2018-10-31 -> 创建文件                          <Tom Free 付瑞彪>
**
**
**       Copyright (c) 深圳市三派智能科技有限公司 All Rights Reserved
**
**       1 Tab == 4 Spaces     UTF-8     ANSI C Language(C99)
*******************************************************************************/

/* 延时函数头文件 */
#include "delay.h"

/*******************************************************************************
** 函数名称：Delay_ms_TimerInit
** 函数作用：初始化Delay ms定时器
** 输入参数：无
** 输出参数：无
** 使用范例：Delay_ms_TimerInit();
** 函数备注：
*******************************************************************************/
#pragma optimize=none
inline static void Delay_ms_TimerInit()
{
    /* 使能TIM5外设时钟 */
    CLK->PCKENR3 |= 0x02;

    /* 128分频，减计数，1ms */
    TIM5_TimeBaseInit(TIM5_Prescaler_128, TIM5_CounterMode_Down, 124);

    /* 清除所有中断标志，防止意外 */
    TIM5->SR1 = 0x00;
    TIM5->SR2 = 0x00;

    /* 清除所有中断，防止意外 */
    TIM5->IER = 0x00;
}

/*******************************************************************************
** 函数名称：Delay_us_TimerInit
** 函数作用：初始化Delay us定时器
** 输入参数：无
** 输出参数：无
** 使用范例：Delay_us_TimerInit();
** 函数备注：
*******************************************************************************/
#pragma optimize=none
inline static void Delay_us_TimerInit(GM_U16 ticks)
{
    /* 使能TIM5外设时钟 */
    CLK->PCKENR3 |= 0x02;

    /* 16分频，减计数 */
    TIM5_TimeBaseInit(TIM5_Prescaler_16, TIM5_CounterMode_Down, ticks);

    /* 清除所有中断标志，防止意外 */
    TIM5->SR1 = 0x00;
    TIM5->SR2 = 0x00;

    /* 关闭所有中断，防止意外 */
    TIM5->IER = 0x00;
}

/*******************************************************************************
** 函数名称：Delay_ms
** 函数作用：延时 n ms
** 输入参数：ms - 延时毫秒数
** 输出参数：无
** 使用范例：Delay_ms(100);
** 函数备注：
*******************************************************************************/
#pragma optimize=none
void Delay_ms(GM_U16 ms)
{
    /* 进行定时器初始化，防止TIM5计数器残留值影响精度 */
    Delay_ms_TimerInit();

    /* 启动TIM5 */
    TIM5->CR1 |= TIM_CR1_CEN;

    while (ms)
    {
        /* 等待时间到 */
        if ((TIM5->SR1 & 0x01))
        {
            TIM5_ClearITPendingBit(TIM5_IT_Update);
            ms--;
        }
    }

    /* 如果时间到，关闭TIM5 */
    TIM5->CR1 &= (uint8_t)(~TIM_CR1_CEN);
}

/*******************************************************************************
** 函数名称：Delay_us
** 函数作用：延时 n us
** 输入参数：us - 延时微秒数，不可超过65535，不可低于25
** 输出参数：无
** 使用范例：Delay_us(100);
** 函数备注：必须保证延时us数大于25，低压25建议采用NOP或者使用NOP的延时函数
*******************************************************************************/
#pragma optimize=none
void Delay_us(GM_U16 us)
{
    /* 防意外 */
    if (us < 21)
    {
        us = 21;
    }
    /* 进行定时器初始化，防止TIM5计数器残留值影响精度 */
    Delay_us_TimerInit(us - 20);

    /* 启动TIM5 */
    TIM5->CR1 |= TIM_CR1_CEN;

    /* 等待时间到 */
    while (!(TIM5->SR1 & 0x01));
    TIM5_ClearITPendingBit(TIM5_IT_Update);

    /* 如果时间到，关闭TIM5 */
    TIM5->CR1 &= ~TIM_CR1_CEN;
}

/*******************************************************************************
** 函数名称：Delay_us_nop
** 函数作用：延时 n us
** 输入参数：us - 延时微秒数，采用nop方式
** 输出参数：无
** 使用范例：Delay_us_nop(5);
** 函数备注：此函数建议在需要延时30us以下时采用
*******************************************************************************/
#pragma optimize=none
void Delay_us_nop(GM_U16 us)
{
    while (us--)
    {
        __no_operation();
        __no_operation();
        __no_operation();
        __no_operation();
        __no_operation();
        __no_operation();
        __no_operation();
        __no_operation();
        __no_operation();
        //__no_operation();
    }
}
