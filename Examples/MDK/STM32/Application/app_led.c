/*******************************************************************************
** 文件名称：app_led.c
** 文件作用：LED灯
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

#include "app_led.h"
#include "hal_gpio_bitband.h"
#include "gm_timer.h"

/* LED位带使用端口 */
#define LED_PORT        C
#define LED_PIN         13
/* 外设库使用端口 */
#define LED_HAL_PORT    GPIOC
#define LED_HAL_PIN     GPIO_Pin_13

static GM_TIMER timer_led;

/*******************************************************************************
** 函数名称：LED_Timer_CallBack
** 函数作用：定时器回调
** 输入参数：无
** 输出参数：无
** 使用样例：LED_Timer_CallBack();
** 函数备注：
*******************************************************************************/
static void LED_Timer_CallBack(void)
{
    LED_Blink();
}

/*******************************************************************************
** 函数名称：LED_Init
** 函数作用：初始化LED
** 输入参数：
** 输出参数：
** 使用样例：LED_Init();
** 函数备注：
*******************************************************************************/
void LED_Init(void)
{
    /* GPIO端口设置 */
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 使能USART1，GPIOA时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    /* LED PC13 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    /* 灭灯 */
    GPIO_SetBits(GPIOC, GPIO_Pin_13);

    GM_Timer_Init(&timer_led, GM_TIMER_MODE_FOREVER, 0, 500, (GM_CALLBACK)LED_Timer_CallBack);
    GM_Timer_Start(&timer_led);
}

/*******************************************************************************
** 函数名称：LED_Control
** 函数作用：LED控制
** 输入参数：isOn - 是否点亮，GM_TRUE - 亮，GM_FALSE - 灭
** 输出参数：无
** 使用样例：LED_Control(GM_TRUE);
** 函数备注：
*******************************************************************************/
void LED_Control(GM_BOOL isOn)
{
    if (isOn == GM_TRUE)
    {
        GPIO_ResetBits(LED_HAL_PORT, LED_HAL_PIN);
    }
    else
    {
        GPIO_SetBits(LED_HAL_PORT, LED_HAL_PIN);
    }
}

/*******************************************************************************
** 函数名称：LED_Control_Fast
** 函数作用：LED快速控制，采用位带
** 输入参数：isOn - 是否点亮，GM_TRUE - 亮，GM_FALSE - 灭
** 输出参数：无
** 使用样例：LED_Control_Fast(GM_FALSE);
** 函数备注：
*******************************************************************************/
void LED_Control_Fast(GM_BOOL isOn)
{
    GPIO_OUT(LED_PORT, LED_PIN) = (isOn == GM_TRUE) ? 0 : 1;
}

/*******************************************************************************
** 函数名称：LED_Blink
** 函数作用：闪烁LED
** 输入参数：无
** 输出参数：无
** 使用样例：LED_Blink();
** 函数备注：
*******************************************************************************/
void LED_Blink(void)
{
    GPIO_OUT(LED_PORT, LED_PIN) ^= 1;
}
