/*******************************************************************************
** 文件名称：hal_timer3.c
** 文件作用：TIMER3
** 编写作者：Tom Free 付瑞彪
** 编写时间：2020-06-22
** 文件备注：
**			 
** 更新记录：
**          2020-06-22 -> 创建文件                             <Tom Free 付瑞彪>
**
**              Copyright (c) 2018-2020 付瑞彪 All Rights Reserved
**
**       1 Tab == 4 Spaces     UTF-8     ANSI C Language(C99)
*******************************************************************************/

#include "hal_timer3.h"
#include "stm32f10x.h"

/* 回调钩子函数 */
static void(*tim3_int_hook)(void) = NULL;

/*******************************************************************************
** 函数名称：HAL_TIM3_Init
** 函数作用：通用定时器3中断初始化
** 输入参数：arr - 自动重装值。
**           psc - 时钟预分频数
** 输出参数：无
** 使用样例：HAL_TIM3_Init(4999,7199);
** 函数备注：这里时钟选择为APB1的2倍，而APB1为36M
*******************************************************************************/
void HAL_TIM3_Init(uint16_t arr, uint16_t psc, void(*hook)())
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 时钟使能 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* 定时器TIM3初始化 */
    /* 设置在下一个更新事件装入活动的自动重装载寄存器周期的值 */
    TIM_TimeBaseStructure.TIM_Period = arr;
    /* 设置用来作为TIMx时钟频率除数的预分频值 */
    TIM_TimeBaseStructure.TIM_Prescaler = psc;
    /* 设置时钟分割:TDTS = Tck_tim */
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    /* TIM向上计数模式 */
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    /* 根据指定的参数初始化TIMx的时间基数单位 */
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    /* 使能指定的TIM3中断,允许更新中断 */
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    /* 中断优先级NVIC设置 */
    /* TIM3中断 */
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    /* 先占优先级0级 */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    /* 从优先级3级 */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    /* IRQ通道被使能 */
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    /* 初始化NVIC寄存器 */
    NVIC_Init(&NVIC_InitStructure);

    /* 使能TIMx */
    TIM_Cmd(TIM3, ENABLE);         

    /* 回调函数 */
    tim3_int_hook = hook;
}

/*******************************************************************************
** 函数名称：TIM3_IRQHandler
** 函数作用：定时器3中断服务程序
** 输入参数：无
** 输出参数：无
** 使用样例：TIM3_IRQHandler();
** 函数备注：
*******************************************************************************/
void TIM3_IRQHandler(void)
{
    /* 检查TIM3更新中断发生与否 */
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        /* 清除TIMx更新中断标志  */
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        /* 执行钩子函数 */
        if (tim3_int_hook != NULL)
        {
            tim3_int_hook();
        }
    }
}
