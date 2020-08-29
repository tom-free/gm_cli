/*******************************************************************************
** 文件名称：hal_uart1.c
** 文件作用：UART1
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

#include "hal_uart1.h"
#include "stm32f10x.h"

/* 回调钩子函数 */
static void(*uart1_int_rx_hook)(uint8_t) = NULL;

/*******************************************************************************
** 函数名称：HAL_UART1_Init
** 函数作用：初始化UART1
** 输入参数：baud - 波特率
**           rx_hook - 接收中断钩子函数
** 输出参数：无
** 使用范例：HAL_UART1_Init(115200ul, hook);
** 函数备注：
*******************************************************************************/
void HAL_UART1_Init(uint32_t baud, void(*rx_hook)(uint8_t))
{
    /* GPIO端口设置 */
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 使能USART1，GPIOA时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    /* USART1_TX GPIOA.9 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART1_RX GPIOA.1初始化 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART1 初始化设置 */
    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    /* 使能 USART1 中断 */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 使能接受中断，在接受移位 寄存器中有数据是产生 */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    USART_Cmd(USART1, ENABLE);

    /* 钩子函数 */
    uart1_int_rx_hook = rx_hook;
}

/*******************************************************************************
** 函数名称：HAL_UART1_PutChar
** 函数作用：输出一个字符
** 输入参数：ch - 输出字符
** 输出参数：无
** 使用范例：HAL_UART1_PutChar('c');
** 函数备注：
*******************************************************************************/
void HAL_UART1_PutChar(char ch)
{
    /* 等待上一次串口数据发送完成 */
    while ((USART1->SR & 0X40) == 0);
    /* 写DR,串口1将发送数据 */
    USART1->DR = ch;
}

/*******************************************************************************
** 函数名称：HAL_UART1_GetChar
** 函数作用：输入一个字符
** 输入参数：无
** 输出参数：字符
** 使用范例：HAL_UART1_GetChar();
** 函数备注：此函数不可在开启接收中断时使用
*******************************************************************************/
char HAL_UART1_GetChar(void)
{
    /* 等待串口输入数据 */
    while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
    return (char)USART_ReceiveData(USART1);
}

/*******************************************************************************
** 函数名称：USART1_IRQHandler
** 函数作用：中断服务函数
** 输入参数：无
** 输出参数：无
** 使用样例：USART1_IRQHandler
** 函数备注：
*******************************************************************************/
void USART1_IRQHandler(void)
{
    uint8_t data;

    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        /* 读取数据并清除中断标志 */
        data = USART_ReceiveData(USART1);
        /* 执行钩子函数 */
        if (uart1_int_rx_hook != NULL)
        {
            uart1_int_rx_hook(data);
        }
    }
}
