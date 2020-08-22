/*******************************************************************************
** 文件名称：hard_uart.c
** 文件作用：硬件串口
** 编写作者：Tom Free 付瑞彪
** 编写时间：2019-11-18
** 文件备注：硬件串口操作，包括UART1、UART2、UART3
**
**
** 更新记录：
**          2019-11-18 -> 创建文件                          <Tom Free 付瑞彪>
**
**
**       Copyright (c) 深圳市三派智能科技有限公司 All Rights Reserved
**
**       1 Tab == 4 Spaces     UTF-8     ANSI C Language(C99)
*******************************************************************************/

#include "hard_uart.h"
#include "delay.h"
#include "stdarg.h"
#include "stdio.h"

/* UART发送最大延时，延时时间基准为10us */
#define UART_SEND_MAX_DELAY         250U

/* UART延时函数 */
#define UART_Delay_us(x)            Delay_us_nop(x)

/*******************************************************************************
** 函数名称：UART_Init
** 函数作用：初始化UART口
** 输入参数：UARTx - UART端口号，可取UART1，UART2，UART3
**           baud  - 波特率
** 输出参数：无
** 使用范例：UART_Init(UART1, 115200);
** 函数备注：此函数使能了接收中断
*******************************************************************************/
void UART_Init(UART_NUM UARTx, GM_U32 baud)
{
    if (UART1 == UARTx)
    {
        /* 使能外部上拉 */
        GPIO_ExternalPullUpConfig(UART1_RX_PORT, UART1_RX_PIN, ENABLE);
        GPIO_ExternalPullUpConfig(UART1_TX_PORT, UART1_TX_PIN, ENABLE);

        /* RX1输入上拉 */
        GPIO_Init(UART1_RX_PORT, UART1_RX_PIN, GPIO_Mode_In_PU_No_IT);
        /* TX1输出高电平 */
        GPIO_Init(UART1_TX_PORT, UART1_TX_PIN, GPIO_Mode_Out_PP_High_Fast);

        /* 开启USART1时钟 */
        CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);

        /* 设置USART1参数，8N1，接收/发送 */
        USART_Init(USART1, baud, USART_WordLength_8b,
                   USART_StopBits_1, USART_Parity_No,
                   (USART_Mode_TypeDef)(USART_Mode_Tx | USART_Mode_Rx));

        /* 使能接收中断 */
        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

        /* 使能USART1 */
        USART_Cmd(USART1, ENABLE);
    }
    else if (UART2 == UARTx)
    {
        /* 使能外部上拉 */
        GPIO_ExternalPullUpConfig(UART2_RX_PORT, UART2_RX_PIN, ENABLE);
        GPIO_ExternalPullUpConfig(UART2_TX_PORT, UART2_TX_PIN, ENABLE);

        /* RX2输入上拉 */
        GPIO_Init(UART2_RX_PORT, UART2_RX_PIN, GPIO_Mode_In_PU_No_IT);
        /* TX2输出高电平 */
        GPIO_Init(UART2_TX_PORT, UART2_TX_PIN, GPIO_Mode_Out_PP_High_Fast);

        /* 开启USART2时钟 */
        CLK_PeripheralClockConfig(CLK_Peripheral_USART2, ENABLE);

        /* 设置USART2参数，8N1，接收/发送 */
        USART_Init(USART2, baud, USART_WordLength_8b,
                   USART_StopBits_2, USART_Parity_No,
                   (USART_Mode_TypeDef)(USART_Mode_Tx | USART_Mode_Rx));

        /* 使能接收中断 */
        USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

        /* 使能USART2 */
        USART_Cmd(USART2, ENABLE);
    }
    else if (UART3 == UARTx)
    {
        /* 使能外部上拉 */
        GPIO_ExternalPullUpConfig(UART3_RX_PORT, UART3_RX_PIN, ENABLE);
        GPIO_ExternalPullUpConfig(UART3_TX_PORT, UART3_TX_PIN, ENABLE);

        /* RX3输入上拉 */
        GPIO_Init(UART3_RX_PORT, UART3_RX_PIN, GPIO_Mode_In_PU_No_IT);
        /* TX3输出高电平 */
        GPIO_Init(UART3_TX_PORT, UART3_TX_PIN, GPIO_Mode_Out_PP_High_Fast);

        /* 开启USART3时钟 */
        CLK_PeripheralClockConfig(CLK_Peripheral_USART3, ENABLE);

        /* 设置USART3参数，8N1，接收/发送 */
        USART_Init(USART3, baud, USART_WordLength_8b,
                   USART_StopBits_1, USART_Parity_No,
                   (USART_Mode_TypeDef)(USART_Mode_Tx | USART_Mode_Rx));

        /* 使能接收中断 */
        USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

        /* 使能USART3 */
        USART_Cmd(USART3, ENABLE);
    }
}

/*******************************************************************************
** 函数名称：UART_UnInit
** 函数作用：UART恢复出厂初始化
** 输入参数：UARTx - UART端口号，可取UART1，UART2，UART3
** 输出参数：无
** 使用范例：UART_UnInit(UART1);
** 函数备注：
*******************************************************************************/
void UART_UnInit(UART_NUM UARTx)
{
    if (UART1 == UARTx)
    {
        USART_Cmd(USART1, DISABLE);
        USART_DeInit(USART1);
    }
    else if (UART2 == UARTx)
    {
        USART_Cmd(USART2, DISABLE);
        USART_DeInit(USART2);
    }
    else if (UART3 == UARTx)
    {
        USART_Cmd(USART3, DISABLE);
        USART_DeInit(USART3);
    }
}

/*******************************************************************************
** 函数名称：UART_WriteByte
** 函数作用：UART发送字节数据
** 输入参数：UARTx - UART端口号，可取UART1，UART2，UART3
**           data  - 待发送数据
** 输出参数：发送是否成功，GM_ERR - 失败，GM_OK - 成功
** 使用范例：GM_BOOL res = UART_WriteByte(UART1, 0x88);
** 函数备注：
*******************************************************************************/
GM_BOOL UART_WriteByte(UART_NUM UARTx, GM_U8 data)
{
    GM_U8   AutoExit = UART_SEND_MAX_DELAY;
    GM_BOOL res = GM_OK;

    if (UART1 == UARTx)
    {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) != SET)
        {
            if (--AutoExit == 0)
            {
                res = GM_ERR;
                goto EXIT;
            }
            UART_Delay_us(10);
        }

        USART1->DR = data;

        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
        {
            if (--AutoExit == 0)
            {
                res = GM_ERR;
                break;
            }
            UART_Delay_us(10);
        }

    }
    else if (UART2 == UARTx)
    {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET)
        {
            if (--AutoExit == 0)
            {
                res = GM_ERR;
                goto EXIT;
            }
            UART_Delay_us(10);
        }

        USART2->DR = data;

        while (USART_GetFlagStatus(USART2, USART_FLAG_TC) != SET)
        {
            if (--AutoExit == 0)
            {
                res = GM_ERR;
                break;
            }
            UART_Delay_us(10);
        }

    }
    else
    {
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) != SET)
        {
            if (--AutoExit == 0)
            {
                res = GM_ERR;
                goto EXIT;
            }
            UART_Delay_us(10);
        }

        USART3->DR = data;

        while (USART_GetFlagStatus(USART3, USART_FLAG_TC) != SET)
        {
            if (--AutoExit == 0)
            {
                res = GM_ERR;
                break;
            }
            UART_Delay_us(10);
        }
    }

EXIT:
    return res;
}

/*******************************************************************************
** 函数名称：UART_WriteNbytes
** 函数作用：UART写N个字节数据
** 输入参数：UARTx - 端口号
**           buf - 发送数据缓冲区
**           len - 数据字节数
** 输出参数：发送是否成功，GM_ERR - 失败，GM_OK - 成功
** 使用范例：GM_BOOL res = UART_WriteNbytes(UART3, buf, 10);
** 函数备注：
*******************************************************************************/
GM_BOOL UART_WriteNbytes(UART_NUM UARTx, GM_U8* buf, GM_U8 len)
{
    GM_BOOL res = GM_OK;

    while (len--)
    {
        res = UART_WriteByte(UARTx, *buf++);
        if (res == GM_ERR)
        {
            break;
        }
    }
    return res;
}

/*******************************************************************************
** 函数名称：UART_PutString
** 函数作用：UART发送字符串
** 输入参数：UARTx - UART端口号，可取UART1，UART2，UART3
**           str  - 待发送字符串地址
** 输出参数：发送是否成功，GM_ERR - 失败，GM_OK - 成功
** 使用范例：GM_BOOL res = UART_PutString(UART1, "Hello\r\n");
** 函数备注：
*******************************************************************************/
GM_BOOL UART_PutString(UART_NUM UARTx, char* str)
{
    GM_BOOL res = GM_OK;
    while (*str)
    {
        res = UART_WriteByte(UARTx, (GM_U8)(*str));
        if (res == GM_ERR)
        {
            break;
        }
        str++;
    }
    return res;
}

/*******************************************************************************
** 函数名称：UART_Printf
** 函数作用：UART发送字符串的printf实现
** 输入参数：UARTx - UART端口号，可取UART1，UART2，UART3
**           fmt  - 待发送字符串格式串
** 输出参数：发送是否成功，GM_ERR - 失败，GM_OK - 成功
** 使用范例：GM_BOOL res = UART_Printf(UART1, "%d,%f\r\n", 12, 1.3);
** 函数备注：调试时使用，不要用于发布的代码
*******************************************************************************/
GM_BOOL UART_Printf(UART_NUM UARTx, const char *fmt,...)
{
    GM_BOOL res = GM_OK;
    va_list ap;
    char string[64];

    va_start(ap, fmt);
    vsprintf(string, fmt, ap);
    res = UART_PutString(UARTx, string);
    va_end(ap);
    return res;
}
