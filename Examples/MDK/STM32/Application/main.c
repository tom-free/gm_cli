/*******************************************************************************
** 文件名称：main.c
** 文件作用：主函数文件
** 编写作者：Tom Free 付瑞彪
** 编写时间：2020-06-21
** 文件备注：此文件是多功能按键的头文件，主要包括API声明和数据类型定义
**			 
** 更新记录：
**          2020-06-21 -> 创建文件                             <Tom Free 付瑞彪>
**
**              Copyright (c) 2018-2020 付瑞彪 All Rights Reserved
**
**       1 Tab == 4 Spaces     UTF-8     ANSI C Language(C99)
*******************************************************************************/

#include "stm32f10x.h"
#include "gm_queue.h"
#include "gm_multi_key.h"
#include "app_led.h"
#include "app_uart1.h"
#include "app_timer3.h"
#include "gm_timer.h"
#include "app_key.h"
#include "gm_cli.h"

/* 主函数 */
int main(int argc, char* argv[])
{
    /* 设置系统中断优先级分组2，四个抢占优先级，四个响应优先级 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    /* 初始化定时器 */
    TIMER3_Init();

    /* 初始化串口 */
    UART1_Init();

    /* 初始化LED */
    LED_Init();

    /* 初始化按键 */
    Key_Init();
    
    /* 初始化CLI */
    GM_CLI_Init();
    /* 注册输出字符驱动 */
    GM_CLI_RegOutCharCallBack((GM_CLI_OUT_CHAR_CB)UART1_PutChar);
    /* 设置命令提示符 */
    GM_CLI_SetCommandPrompt("[STM32F103C8T6] > ");
    /* 启动CLI */
    GM_CLI_Start();
    
    for (;;)
    {
        if (UART1_HasData() == GM_TRUE)
        {
            GM_CLI_ParseOneChar(UART1_GetChar());
            //UART1_Printf("%c", UART1_GetChar());
        }

        GM_Timer_PollEvent();
    }
}
