/*******************************************************************************
** 文件名称：uart.h
** 文件作用：USART操作头文件
** 编写作者：Tom Free 付瑞彪
** 编写时间：2018-11-03
** 文件备注：
**
**
** 更新记录：
**          2018-11-03 -> 创建文件                          <Tom Free 付瑞彪>
**
**
**       Copyright (c) 深圳市三派智能科技有限公司 All Rights Reserved
**
**       1 Tab == 4 Spaces     UTF-8     ANSI C Language(C99)
*******************************************************************************/
#ifndef __HARD_UART_H__
#define __HARD_UART_H__

/* 数据类型定义 */
#include "typedef.h"

/* USART组定义 */
typedef enum
{
    UART1 = 0x00,
    UART2 = 0x01,
    UART3 = 0x02
} UART_NUM;

/******************************************************************************/
/*************************** 单片机串口端口定义 *******************************/
/* UART1 */
#define UART1_RX_PORT           GPIOC
#define UART1_RX_PIN            GPIO_Pin_2
#define UART1_RX_EXTI_PIN       EXTI_Pin_2
#define UART1_RX_EXTI_IT        EXTI_IT_Pin2

#define UART1_TX_PORT           GPIOC
#define UART1_TX_PIN            GPIO_Pin_3

/* UART2 */
#define UART2_RX_PORT           GPIOE
#define UART2_RX_PIN            GPIO_Pin_3
#define UART2_RX_EXTI_PIN       EXTI_Pin_3
#define UART2_RX_EXTI_IT        EXTI_IT_Pin3

#define UART2_TX_PORT           GPIOE
#define UART2_TX_PIN            GPIO_Pin_4

/* UART3 */
#define UART3_RX_PORT           GPIOF
#define UART3_RX_PIN            GPIO_Pin_1
#define UART3_RX_EXTI_PIN       EXTI_Pin_1
#define UART3_RX_EXTI_IT        EXTI_IT_Pin1

#define UART3_TX_PORT           GPIOF
#define UART3_TX_PIN            GPIO_Pin_0

#define UART_IO_IN_PU(n, T)     GPIO_Init((n ## _ ## T ## _PORT), (n ## _ ## T ## _PIN), GPIO_Mode_In_PU_No_IT)
#define UART_IO_IN_PU_INT(n, T) GPIO_Init((n ## _ ## T ## _PORT), (n ## _ ## T ## _PIN), GPIO_Mode_In_PU_IT)
#define UART_IO_SET_OUT_H(n, T) GPIO_Init((n ## _ ## T ## _PORT), (n ## _ ## T ## _PIN), GPIO_Mode_Out_PP_High_Slow)
#define UART_IO_SET_OUT_L(n, T) GPIO_Init((n ## _ ## T ## _PORT), (n ## _ ## T ## _PIN), GPIO_Mode_Out_PP_Low_Slow)
#define UART_IO_OUT_H(n, T)     (n ## _ ## T ## _PORT->ODR) |= (uint8_t)(n ## _ ## T ## _PIN)
#define UART_IO_OUT_L(n, T)     (n ## _ ## T ## _PORT->ODR) &= (uint8_t)(~(n ## _ ## T ## _PIN))
#define UART_IO_IN_READ(n, T)   ((n ## _ ## T ## _PORT->IDR) & (n ## _ ## T ## _PIN))
#define UART_IO_INT_FALL(n, T)  EXTI_SetPinSensitivity(n ## _ ## T ## _EXTI_PIN, EXTI_Trigger_Falling)
#define UART_IO_CLEAR_IT(n, T)  EXTI_ClearITPendingBit(n ## _ ## T ## _EXTI_IT)

#define UART_RX_IN_PU_INIT(n)       UART_IO_IN_PU(n, RX)
#define UART_RX_IN_PU_INT_INIT(n)   UART_IO_IN_PU_INT(n, RX)
#define UART_RX_OUT_H_INIT(n)       UART_IO_SET_OUT_H(n, RX)
#define UART_RX_OUT_L_INIT(n)       UART_IO_SET_OUT_L(n, RX)
#define UART_RX_OUT_L(n)            UART_IO_OUT_L(n, RX)
#define UART_RX_OUT_H(n)            UART_IO_OUT_H(n, RX)
#define UART_RX_READ(n)             UART_IO_IN_READ(n, RX)
#define UART_RX_CLEAR_IT(n)         UART_IO_CLEAR_IT(n, RX)
#define UART_RX_INT_FALL(n)         UART_IO_INT_FALL(n, RX)

#define UART_TX_IN_PU_INIT(n)       UART_IO_IN_PU(n, TX)
#define UART_TX_OUT_H_INIT(n)       UART_IO_SET_OUT_H(n, TX)
#define UART_TX_OUT_L_INIT(n)       UART_IO_SET_OUT_L(n, TX)
#define UART_TX_OUT_H(n)            UART_IO_OUT_H(n, TX)
#define UART_TX_OUT_L(n)            UART_IO_OUT_L(n, TX)
/********************************** END ***************************************/

/*******************************************************************************
** 函数名称：UART_Init
** 函数作用：初始化UART口
** 输入参数：UARTx - UART端口号，可取UART1，UART2，UART3
**           baud  - 波特率
** 输出参数：无
** 使用范例：UART_Init(UART1, 115200);
** 函数备注：此函数使能了接收中断
*******************************************************************************/
void UART_Init(UART_NUM UARTx, GM_U32 baud);

/*******************************************************************************
** 函数名称：UART_UnInit
** 函数作用：UART恢复出厂初始化
** 输入参数：UARTx - UART端口号，可取UART1，UART2，UART3
** 输出参数：无
** 使用范例：UART_UnInit(UART1);
** 函数备注：此函数使能了接收中断
*******************************************************************************/
void UART_UnInit(UART_NUM UARTx);

/*******************************************************************************
** 函数名称：UART_WriteByte
** 函数作用：UART发送字节数据
** 输入参数：UARTx - UART端口号，可取UART1，UART2，UART3
**           data  - 待发送数据
** 输出参数：发送是否成功，GM_ERR - 失败，GM_OK - 成功
** 使用范例：GM_BOOL res = UART_WriteByte(UART1, 0x88);
** 函数备注：
*******************************************************************************/
GM_BOOL UART_WriteByte(UART_NUM UARTx, GM_U8 data);

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
GM_BOOL UART_WriteNbytes(UART_NUM UARTx, GM_U8* buf, GM_U8 len);

/*******************************************************************************
** 函数名称：UART_PutString
** 函数作用：UART发送字符串
** 输入参数：UARTx - UART端口号，可取UART1，UART2，UART3
**           str  - 待发送字符串地址
** 输出参数：发送是否成功，GM_ERR - 失败，GM_OK - 成功
** 使用范例：GM_BOOL res = UART_PutString(UART1, "Hello\r\n");
** 函数备注：
*******************************************************************************/
GM_BOOL UART_PutString(UART_NUM UARTx, char* str);

/*******************************************************************************
** 函数名称：UART_Printf
** 函数作用：UART发送字符串的printf实现
** 输入参数：UARTx - UART端口号，可取UART1，UART2，UART3
**           fmt  - 待发送字符串格式串
** 输出参数：发送是否成功，GM_ERR - 失败，GM_OK - 成功
** 使用范例：GM_BOOL res = UART_Printf(UART1, "%d,%f\r\n", 12, 1.3);
** 函数备注：调试时使用，不要用于发布的代码
*******************************************************************************/
GM_BOOL UART_Printf(UART_NUM UARTx, const char *fmt, ...);

#endif  /* __HARD_UART_H__ */
