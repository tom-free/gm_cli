/*******************************************************************************
** 文件名称：app_key.c
** 文件作用：按键
** 编写作者：Tom Free 付瑞彪
** 编写时间：2020-06-23
** 文件备注：
**			 
** 更新记录：
**          2020-06-23 -> 创建文件                             <Tom Free 付瑞彪>
**
**              Copyright (c) 2018-2020 付瑞彪 All Rights Reserved
**
**       1 Tab == 4 Spaces     UTF-8     ANSI C Language(C99)
*******************************************************************************/

#include "app_key.h"
#include "hal_gpio_bitband.h"
#include "gm_multi_key.h"
#include "app_uart1.h"
#include "gm_timer.h"

/* 按键位带使用端口 */
#define KEY_PORT        A
#define KEY_PIN         0
/* 外设库使用端口 */
#define KEY_HAL_PORT    GPIOA
#define KEY_HAL_PIN     GPIO_Pin_0

static GM_TIMER timer_key;
static GM_MULTI_KEY key;

/*******************************************************************************
** 函数名称：Key_Read
** 函数作用：按键读取电平
** 输入参数：无
** 输出参数：电平
** 使用样例：Key_Read();
** 函数备注：
*******************************************************************************/
static GM_MULTI_KEY_LEVEL Key_Read(void)
{
    if (GPIO_IN(KEY_PORT, KEY_PIN))
    {
        return GM_MULTI_KEY_LEVEL_RELEASE;
    }
    else
    {
        return GM_MULTI_KEY_LEVEL_PRESS;
    }
}

/*******************************************************************************
** 函数名称：Key_CallBack
** 函数作用：按键回调函数
** 输入参数：通用参数
** 输出参数：结果
** 使用样例：Key_CallBack();
** 函数备注：
*******************************************************************************/
static int Key_CallBack(int argc, char *argv[])
{
    GM_MULTI_KEY* pkey = (GM_MULTI_KEY*)argv;

    switch (pkey->event)
    {
    case GM_MULTI_KEY_EVENT_PRESS:
        UART1_Printf("key press\r\n");
        break;

    case GM_MULTI_KEY_EVENT_RELEASE:
        UART1_Printf("key release\r\n");
        break;

    case GM_MULTI_KEY_EVENT_SINGLE_CLICK:
        UART1_Printf("key single click\r\n");
        break;

    case GM_MULTI_KEY_EVENT_DOUBLE_CLICK:
        UART1_Printf("key double click\r\n");
        break;

    case GM_MULTI_KEY_EVENT_MULTI_CLICK:
        UART1_Printf("key %d click\r\n", pkey->click_cnt);
        break;

    case GM_MULTI_KEY_EVENT_LONG_FIRST:
        UART1_Printf("key long press\r\n", pkey->click_cnt);
        break;

    case GM_MULTI_KEY_EVENT_REPEAT:
        UART1_Printf("key repeat press\r\n", pkey->click_cnt);
        break;

    default:
        break;
    }
    
    return 0;
}

/*******************************************************************************
** 函数名称：Key_Init
** 函数作用：初始化按键
** 输入参数：无
** 输出参数：无
** 使用样例：Key_Init();
** 函数备注：
*******************************************************************************/
void Key_Init(void)
{
    /* GPIO端口设置 */
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 使能GPIOA时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* KEY PA0 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GM_MultiKey_Init(&key, Key_Read, Key_CallBack);
    GM_MultiKey_Add(&key);

    GM_Timer_Init(&timer_key, GM_TIMER_MODE_FOREVER, 0, 5, (GM_CALLBACK)GM_MultiKey_PollEvent);
    GM_Timer_Start(&timer_key);
}
