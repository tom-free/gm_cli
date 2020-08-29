/*******************************************************************************
** 文件名称：gm_multi_timer.h
** 文件作用：多功能定时器
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
#ifndef __GM_MULTI_TIMER_H__
#define __GM_MULTI_TIMER_H__

#include "gm_type.h"

/* 整数定义 */
typedef GM_U32 GM_TIMER_INT;

/* 定时器模式 */
typedef enum
{
    GM_TIMER_MODE_ONCE,     /* 单次模式 */
    GM_TIMER_MODE_MANY,     /* 多次模式 */
    GM_TIMER_MODE_FOREVER,  /* 无限循环模式 */
} GM_TIMER_MODE;

/* 定时器状态 */
typedef enum
{
    GM_TIMER_STATUS_PENDING,    /* 挂起态 */
    GM_TIMER_STATUS_RUNNING,    /* 运行态 */
} GM_TIMER_STATUS;

/* 定时器结构 */
typedef struct GM_TIMER
{
    GM_TIMER_MODE    mode;          /* 定时器模式 */
    GM_TIMER_STATUS  status;        /* 定时器状态 */
    GM_TIMER_INT     times;         /* 重载次数，仅多次模式下使用 */
    GM_TIMER_INT     interval;      /* 计时周期，单位ticks */
    GM_TIMER_INT     last;          /* 上一次计数溢出值 */
    GM_CALLBACK      cb;            /* 溢出回调函数 */
    struct GM_TIMER *next;          /* 下一个节点指针，用户不能摆弄，否则gg */
} GM_TIMER;

#ifdef __cplusplus  
extern "C" {  
#endif  /* __cplusplus */

/*******************************************************************************
** 函数名称：GM_Timer_Init
** 函数作用：初始化
** 输入参数：ptimer - 定时器指针
**           mode - 模式
**           times - 运行次数，仅多次模式下有效
**           interval - 触发时间间隔
**           cb - 回调
** 输出参数：是否成功
** 使用样例：GM_Timer_Init(&tim, GM_TIMER_MODE_ONCE, 0, 100, GM_NULL);
** 函数备注：此函数会将初始化后的定时器插入系统链表，但不会启动定时器
*******************************************************************************/
GM_BOOL GM_Timer_Init(GM_TIMER* ptimer, GM_TIMER_MODE mode, GM_U32 times, GM_U32 interval, GM_CALLBACK cb);

/*******************************************************************************
** 函数名称：GM_Timer_Remove
** 函数作用：从链表移除定时器
** 输入参数：ptimer - 定时器指针
** 输出参数：是否成功
** 使用样例：GM_Timer_Remove(ptimer);
** 函数备注：定时器非法或不在链表会返回失败
*******************************************************************************/
GM_BOOL GM_Timer_Remove(GM_TIMER* ptimer);

/*******************************************************************************
** 函数名称：GM_Timer_Start
** 函数作用：启动定时器
** 输入参数：ptimer - 定时器指针
** 输出参数：无
** 使用样例：GM_Timer_Start();
** 函数备注：
*******************************************************************************/
GM_BOOL GM_Timer_Start(GM_TIMER* ptimer);

/*******************************************************************************
** 函数名称：GM_Timer_Stop
** 函数作用：停止定时器
** 输入参数：ptimer - 定时器指针
** 输出参数：无
** 使用样例：GM_Timer_Stop();
** 函数备注：
*******************************************************************************/
GM_BOOL GM_Timer_Stop(GM_TIMER* ptimer);

/*******************************************************************************
** 函数名称：GM_Timer_PollEvent
** 函数作用：定时器事件轮询处理
** 输入参数：无
** 输出参数：无
** 使用样例：GM_Timer_PollEvent();
** 函数备注：
*******************************************************************************/
void GM_Timer_PollEvent(void);

/*******************************************************************************
** 函数名称：GM_Timer_GetTicks
** 函数作用：获取系统ticks总数
** 输入参数：无
** 输出参数：ticks
** 使用样例：GM_Timer_GetTicks();
** 函数备注：
*******************************************************************************/
GM_TIMER_INT GM_Timer_GetTicks(void);

/*******************************************************************************
** 函数名称：GM_Timer_IncTicks
** 函数作用：定时器ticks自增
** 输入参数：无
** 输出参数：无
** 使用样例：GM_Timer_IncTicks();
** 函数备注：此函数一般放在硬件定时器定时中断中执行，定时周期推荐1ms
*******************************************************************************/
void GM_Timer_IncTicks(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* __GM_MULTI_TIMER_H__ */
