/*******************************************************************************
** 文件名称：platform.c
** 文件作用：单片机平台文件
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

#include "platform.h"

/* STM8L ID存放地址 */
#define STM8L_UID_BASE_ADDR     0x4926U
/* STM8L ID长度 */
#define STM8L_UID_LENGTH        12U

/*******************************************************************************
** 函数名称：STM8L_GetUID
** 函数作用：获取STM8L芯片ID号
** 输入参数：pUID - 接收UID缓冲区地址
** 输出参数：是否成功，GM_OK - 成功
** 使用范例：GetUniqueID();
** 函数备注：
*******************************************************************************/
void STM8L_GetUID(GM_U8 *pUID)
{
    GM_U8 i = 0;

    /* ID起始地址 */
    GM_U8 *pIDStart = (GM_U8 *)(STM8L_UID_BASE_ADDR);

    for (i = 0; i < STM8L_UID_LENGTH; i++)
    {
        *pUID++ = *pIDStart++;
    }
}

/*******************************************************************************
** 函数名称：STM8L_ChangeMCLK
** 函数作用：设置STM8L系统主时钟
** 输入参数：mclk - 系统时钟类型
**           MCLK_INT_16MHZ  - 内部16MHZ
**           MCLK_EXT_8MHZ   - 外部8MHZ
**           MCLK_LOW_128KHZ - 内部128KHZ
** 输出参数：无
** 使用范例：GM_BOOL res = STM8L_ChangeMCLK(MCLK_INT_16MHZ);
** 函数备注：此函数如果时钟源异常会出现死等的情况
*******************************************************************************/
void STM8L_ChangeMCLK(MCLK_SOURCE mclk)
{
    if (MCLK_INT_16MHZ == mclk)
    {
        /* 使能内部高速RC时钟，上电默认开启 */
        CLK_HSICmd(ENABLE);
        /* 将系统时钟源配置为内部高速RC时钟 */
        CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);
        /* 等待内部时钟准备好 */
        while (CLK_GetFlagStatus(CLK_FLAG_HSIRDY) == RESET);
        /* 系统时钟不分频 */
        CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
    }
    else if (MCLK_INT_1MHZ == mclk)
    {
        /* 使能内部高速RC时钟，上电默认开启 */
        CLK_HSICmd(ENABLE);
        /* 将系统时钟源配置为内部高速RC时钟 */
        CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);
        /* 等待内部时钟准备好 */
        while (CLK_GetFlagStatus(CLK_FLAG_HSIRDY) == RESET);
        /* 系统时钟16分频 */
        CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_16);
    }
    else if (MCLK_EXT_8MHZ == mclk)
    {
        /* 开启外部高速时钟源 */
        CLK_HSEConfig(CLK_HSE_ON);
        /* 等待HSE准备就绪 */
        while (CLK_GetFlagStatus(CLK_FLAG_HSERDY) == RESET);
        /* 将系统时钟源配置为外部高速时钟 */
        CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSE);
        /* 进行时钟切换 */
        CLK_SYSCLKSourceSwitchCmd(ENABLE);
        /* 系统时钟2分频 */
        CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_2);
    }
    else if (MCLK_LOW_128KHZ == mclk)
    {
        /* 使能内部低速时钟源 */
        CLK_LSICmd(ENABLE);
        /* 等待LSI准备就绪 */
        while (CLK_GetFlagStatus(CLK_FLAG_LSIRDY) == RESET);
        /* 将系统时钟源配置为内部低速时钟 */
        CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_LSI);
        /* 进行时钟切换 */
        CLK_SYSCLKSourceSwitchCmd(ENABLE);
        /* 系统时钟2分频 */
        CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
    }
}
