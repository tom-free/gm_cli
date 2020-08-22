/*******************************************************************************
** 文件名称：main.c
** 文件作用：主函数文件
** 编写作者：Tom Free 付瑞彪
** 编写时间：2020-08-09
** 文件备注：
**
** 更新记录：
**          2020-08-09 -> 创建文件                             <Tom Free 付瑞彪>
**
**              Copyright (c) 2018-2020 付瑞彪 All Rights Reserved
**
**       1 Tab == 4 Spaces     UTF-8     ANSI C Language(C99)
*******************************************************************************/

#include "stdio.h"
#include "gm_cli.h"
#include "typedef.h"
#include "hard_uart.h"
#include "platform.h"
#include "queue.h"

/* 串口3队列 */
QUEUE uart3_queue;
static GM_U8 uart3_buf[128];

/*******************************************************************************
** 函数名称：UART3_PutChar
** 函数作用：串口打印字符
** 输入参数：ch - 字符
** 输出参数：无
** 使用范例：UART3_PutChar('c');
** 函数备注：
*******************************************************************************/
void UART3_PutChar(const char ch)
{
    UART_WriteByte(UART3, (GM_U8)ch);
}

/*******************************************************************************
** 函数名称：main
** 函数作用：主函数
** 输入参数：argc - 参数个数，argv - 参数表
** 输出参数：执行结果
** 使用范例：main();
** 函数备注：
*******************************************************************************/
int main(int argc, char* argv[])
{
    char ch;

    disableInterrupts();

    /* 设置时钟 */
    STM8L_ChangeMCLK(MCLK_INT_16MHZ);
    /* 重新定位UART3端口为PF0，PF1 */
    SYSCFG_REMAPPinConfig(REMAP_Pin_USART3TxRxPortF, ENABLE);
    /* 初始化串口 */
    UART_Init(UART3, 115200ul);

    /* 初始化queue */
    Queue_Init(&uart3_queue, uart3_buf, sizeof(uart3_buf));

    enableInterrupts();

    /* 初始化 */
    GM_CLI_Init();
    /* 注册输出驱动 */
    GM_CLI_RegOutCharCallBack((GM_CLI_OUT_CHAR_CB)UART3_PutChar);
    /* 设置提示符 */
    GM_CLI_SetCommandNotice("[STM8L052R8] > ");
    /* 启动CLI */
    GM_CLI_Start();

    for (;;)
    {
        if (Queue_NonEmpty(&uart3_queue) == GM_TRUE)
        {
             if (Queue_Read(&uart3_queue, (GM_U8*)&ch) == GM_TRUE)
             {
                 GM_CLI_ParseOneChar(ch);
             }
        }
    }

    return 0;
}
