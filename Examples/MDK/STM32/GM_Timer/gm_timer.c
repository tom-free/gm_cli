/*******************************************************************************
** 文件名称：gm_multi_timer.c
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

#include "gm_timer.h"

/* 定时器最大定时周期 */
#define GM_TIMER_MAX_INTERVAL   GM_TYPE_UNSIGNED_MAX(GM_TIMER_INT)

/* 定时器控制 */
typedef struct
{
    /* 运行和挂起链表头 */
    GM_TIMER* list_head;
    /* 定时器ticks */
    volatile GM_TIMER_INT ticks;
} GM_TIMER_CTRL;

/* 定时器控制参数 */
static GM_TIMER_CTRL gm_timer_ctrl = 
{
    .list_head = GM_NULL,
    .ticks = 0,
};

/*******************************************************************************
** 函数名称：GM_Timer_AddToList
** 函数作用：添加定时器进链表
** 输入参数：ptimer - 定时器指针
** 输出参数：是否成功
** 使用样例：GM_Timer_AddToList();
** 函数备注：此函数将定时器插入定时器链表头部，需保证指针合法
*******************************************************************************/
static GM_BOOL GM_Timer_AddToList(GM_TIMER* ptimer)
{
    GM_TIMER* ptemp = gm_timer_ctrl.list_head;

    /* 搜索是否已经存在于链表中 */
    while (ptemp != GM_NULL)
    {
        if (ptemp == ptimer)
        {
            /* 已存在 */
            return GM_FALSE;
        }
        ptemp = ptemp->next;
    }

    /* 添加到链表头部 */
    ptimer->next = gm_timer_ctrl.list_head;
    gm_timer_ctrl.list_head = ptimer;

    return GM_TRUE;
}

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
GM_BOOL GM_Timer_Init(GM_TIMER* ptimer, GM_TIMER_MODE mode, GM_U32 times, GM_U32 interval, GM_CALLBACK cb)
{
    if (ptimer == GM_NULL)
    {
        return GM_FALSE;
    }

    if ((interval == 0) ||
        (interval > GM_TIMER_MAX_INTERVAL))
    {
        return GM_FALSE;
    }

    ptimer->mode = mode;
    ptimer->times = times;
    ptimer->next = GM_NULL;
    ptimer->interval = interval;
    ptimer->status = GM_TIMER_STATUS_PENDING;
    ptimer->cb = cb;

    return GM_Timer_AddToList(ptimer);
}

/*******************************************************************************
** 函数名称：GM_Timer_Remove
** 函数作用：从链表移除定时器
** 输入参数：ptimer - 定时器指针
** 输出参数：是否成功
** 使用样例：GM_Timer_Remove(ptimer);
** 函数备注：定时器非法或不在链表会返回失败
*******************************************************************************/
GM_BOOL GM_Timer_Remove(GM_TIMER* ptimer)
{
    GM_TIMER* ptemp = gm_timer_ctrl.list_head;

    if ((ptimer == GM_NULL) || (ptemp == GM_NULL))
    {
        /* 定时器非法 */
        return GM_FALSE;
    }

    /* 搜索链表中timer位置的前一个 */
    while ((ptemp != GM_NULL) && (ptemp->next != ptimer))
    {
        ptemp = ptemp->next;
    }

    if (ptemp == GM_NULL)
    { 
        /* 定时器不在链表中 */
        return GM_FALSE;
    }

    /* 断链 */
    ptemp->next = ptimer->next;
    /* 释放指针，防止别人摆弄 */
    ptimer->next = GM_NULL;

    return GM_TRUE;
}

/*******************************************************************************
** 函数名称：GM_Timer_Start
** 函数作用：启动定时器
** 输入参数：ptimer - 定时器指针
** 输出参数：无
** 使用样例：GM_Timer_Start();
** 函数备注：
*******************************************************************************/
GM_BOOL GM_Timer_Start(GM_TIMER* ptimer)
{
    if (ptimer == GM_NULL)
    {
        /* 定时器非法 */
        return GM_FALSE;
    }
    /* 切换为运行态 */
    ptimer->status = GM_TIMER_STATUS_RUNNING;
    /* 保存当前ticks */
    ptimer->last = gm_timer_ctrl.ticks;

    return GM_TRUE;
}

/*******************************************************************************
** 函数名称：GM_Timer_Stop
** 函数作用：停止定时器
** 输入参数：ptimer - 定时器指针
** 输出参数：无
** 使用样例：GM_Timer_Stop();
** 函数备注：
*******************************************************************************/
GM_BOOL GM_Timer_Stop(GM_TIMER* ptimer)
{
    if (ptimer == GM_NULL)
    {
        /* 定时器非法 */
        return GM_FALSE;
    }
    /* 切换为挂起态 */
    ptimer->status = GM_TIMER_STATUS_PENDING;
    /* 保存当前ticks，用于仿真调试时查看停止时间 */
    ptimer->last = gm_timer_ctrl.ticks;

    return GM_TRUE;
}

/*******************************************************************************
** 函数名称：GM_Timer_PollOneTimer
** 函数作用：轮询一个定时器
** 输入参数：ptimer - 定时器指针
** 输出参数：无
** 使用样例：GM_Timer_PollOneTimer();
** 函数备注：此函数属于内部调用，已保证指针合法且在链表中
*******************************************************************************/
static void GM_Timer_PollOneTimer(GM_TIMER* ptimer)
{
    GM_TIMER_INT delta = gm_timer_ctrl.ticks - ptimer->last;

    if (delta >= ptimer->interval)
    {
        switch (ptimer->mode)
        {
        case GM_TIMER_MODE_ONCE:
            ptimer->status = GM_TIMER_STATUS_PENDING;
            break;

        case GM_TIMER_MODE_MANY:
            if (--ptimer->times)
            {
                ptimer->last = ptimer->last + ptimer->interval;
            }
            else
            {
                ptimer->status = GM_TIMER_STATUS_PENDING;
            }
            break;

        case GM_TIMER_MODE_FOREVER:
            ptimer->last = ptimer->last + ptimer->interval;
            break;

        default:
            break;
        }

        if (ptimer->cb != GM_NULL)
        {
            ptimer->cb(1, (char**)ptimer);
        }
    }
}

/*******************************************************************************
** 函数名称：GM_Timer_PollEvent
** 函数作用：定时器事件轮询处理
** 输入参数：无
** 输出参数：无
** 使用样例：GM_Timer_PollEvent();
** 函数备注：
*******************************************************************************/
void GM_Timer_PollEvent(void)
{
    GM_TIMER* ptemp = gm_timer_ctrl.list_head;
    
    for (; ptemp != GM_NULL; ptemp = ptemp->next)
    {
        if (ptemp->status == GM_TIMER_STATUS_RUNNING)
        {
            GM_Timer_PollOneTimer(ptemp);
        }
    }
}

/*******************************************************************************
** 函数名称：GM_Timer_GetTicks
** 函数作用：获取系统ticks总数
** 输入参数：无
** 输出参数：ticks
** 使用样例：GM_Timer_GetTicks();
** 函数备注：
*******************************************************************************/
GM_TIMER_INT GM_Timer_GetTicks(void)
{
    return gm_timer_ctrl.ticks;
}

/*******************************************************************************
** 函数名称：GM_Timer_IncTicks
** 函数作用：定时器ticks自增
** 输入参数：无
** 输出参数：无
** 使用样例：GM_Timer_IncTicks();
** 函数备注：此函数一般放在硬件定时器定时中断中执行，定时周期推荐1ms
*******************************************************************************/
void GM_Timer_IncTicks(void)
{
    gm_timer_ctrl.ticks++;
}
