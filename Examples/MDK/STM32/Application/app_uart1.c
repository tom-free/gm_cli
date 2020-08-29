/*******************************************************************************
** 文件名称：app_uart1.c
** 文件作用：应用层UART1处理
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

#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "hal_uart1.h"
#include "gm_queue.h"
#include "app_uart1.h"

/* UART1接收队列 */
static GM_QUEUE uart1_rx_queue;

/*******************************************************************************
** 函数名称：UART1_RxHookHandler
** 函数作用：接收钩子函数
** 输入参数：daya - 接收数据
** 输出参数：无
** 使用样例：UART1_RxHookHandler();
** 函数备注：
*******************************************************************************/
static void UART1_RxHookHandler(uint8_t data)
{
    /* 写入数据进队列 */
    GM_Queue_Write(uart1_rx_queue, &data);
}

/*******************************************************************************
** 函数名称：UART1_Init
** 函数作用：初始化串口1
** 输入参数：无
** 输出参数：无
** 使用样例：UART1_Init();
** 函数备注：
*******************************************************************************/
void UART1_Init(void)
{
    /* 创建Queue */
    uart1_rx_queue = GM_Queue_Create(128, sizeof(char));
    /* 初始化硬件串口 */
    HAL_UART1_Init(115200UL, UART1_RxHookHandler);

    UART1_Printf("\033[2J");
    UART1_Printf("\33[0;0H");
    UART1_Printf("Hello World\r\n");
}

/*******************************************************************************
** 函数名称：UART1_PutChar
** 函数作用：输出字符函数
** 输入参数：ch - 字符
** 输出参数：无
** 使用范例：UART1_PutChar('C');
** 函数备注：
*******************************************************************************/
void UART1_PutChar(char ch)
{
    HAL_UART1_PutChar(ch);
}

/*******************************************************************************
** 函数名称：UART1_PutString
** 函数作用：输出字符串函数
** 输入参数：str - 字符串地址
** 输出参数：无
** 使用范例：UART1_PutString("Hello World!");
** 函数备注：
*******************************************************************************/
void UART1_PutString(char *str)
{
    while (*str)
    {
        HAL_UART1_PutChar(*str++);
    }
}

/*******************************************************************************
** 函数名称：HAL_UART1_Printf
** 函数作用：输出一个指定格式的字符串函数
** 输入参数：fmt - 待发送含格式的字符串首地址
** 输出参数：无
** 使用范例：HAL_UART1_PutString("value:%d"，100);
** 函数备注：
*******************************************************************************/
void UART1_Printf(const char *fmt, ...)
{
    static char str[256];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(str, fmt, ap);
    UART1_PutString(str);
    va_end(ap);
}

/*******************************************************************************
** 函数名称：UART1_HasData
** 函数作用：查询UART1是否有数据
** 输入参数：无
** 输出参数：是否存在数据
** 使用样例：UART1_HasData();
** 函数备注：
*******************************************************************************/
GM_BOOL UART1_HasData(void)
{
    return (GM_Queue_Length(uart1_rx_queue) > 0) ? GM_TRUE : GM_FALSE;
}

/*******************************************************************************
** 函数名称：UART1_GetChar
** 函数作用：读取字符
** 输入参数：无
** 输出参数：字符
** 使用样例：UART1_GetChar();
** 函数备注：
*******************************************************************************/
char UART1_GetChar(void)
{
    char c;
    if (GM_Queue_Read(uart1_rx_queue, &c) != GM_TRUE)
    {
        c = ' ';
    }
    return c;
}

/*******************************************************************************
** 函数名称：UART1_Scanf
** 函数作用：输入一个指定格式的字符串函数
** 输入参数：fmt - 待输入含格式的字符串首地址
** 输出参数：参数个数
** 使用范例：UART1_Scanf("%d", &a);
** 函数备注：
*******************************************************************************/
int UART1_Scanf(const char *fmt, ...)
{
    int i = 0;
    unsigned char c;
    va_list args;
    static char str[256];

    while (1)
    {
        /* 从串口接收字符 */
        if (UART1_HasData() == GM_TRUE)
        {
            c = UART1_GetChar();
            HAL_UART1_PutChar(c);
            if ((c == 0x0d) || (c == 0x0a))
            {
                str[i] = ' ';
                str[i + 1] = '\0';
                break;
            }
            else
            {
                str[i++] = c;
            }
        }
    }
    va_start(args, fmt);
    i = vsscanf(str, fmt, args);
    va_end(args);

    UART1_PutString("\r\n");

    return i;
}
