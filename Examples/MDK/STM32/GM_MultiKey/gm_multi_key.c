/*******************************************************************************
** 文件名称：gm_multi_key.c
** 文件作用：多功能按键
** 编写作者：Tom Free 付瑞彪
** 编写时间：20-05-27
** 文件备注：此文件是多功能按键的源文件，主要包括API的具体实现和数据定义
**           
** 更新记录：
**          2020-05-27 -> 创建文件                             <Tom Free 付瑞彪>
**
**              Copyright (c) 2018-2020 付瑞彪 All Rights Reserved
**
**       1 Tab == 4 Spaces     UTF-8     ANSI C Language(C99)
*******************************************************************************/

#include "gm_multi_key.h"
#include "string.h"

/* 轮询检测间隔时间，单位：ms，一般5-20ms为宜，
 * 太长反应迟钝，太快占用CPU时间 */
#ifndef GM_MULTI_KEY_POLL_INTERVAL
#define GM_MULTI_KEY_POLL_INTERVAL      (5u)
#endif  /* GM_MULTI_KEY_POLL_INTERVAL */

/* 防抖动滴答数量，时间等于上门的数乘以数量，一般在10-20ms为宜，
 * 太少消抖不完全，太长反应迟钝 */
#ifndef GM_MULTI_KEY_DEBOUNCE_TICKS
#define GM_MULTI_KEY_DEBOUNCE_TICKS     (3u)
#endif  /* GM_MULTI_KEY_DEBOUNCE_TICKS */

/* 连击最长间隔时间，超过此时间认为连击被终结，换算成滴答数 */
#ifndef GM_MULTI_KEY_HIT_AGIN_TICKS
#define GM_MULTI_KEY_HIT_AGIN_TICKS     (300u / GM_MULTI_KEY_POLL_INTERVAL)
#endif  /* GM_MULTI_KEY_HIT_AGIN_TICKS */

/* 长按触发时间，超过此时间认为进入长按，换算成滴答数 */
#ifndef GM_MULTI_KEY_LONG_TICKS
#define GM_MULTI_KEY_LONG_TICKS         (1000u / GM_MULTI_KEY_POLL_INTERVAL)
#endif  /* GM_MULTI_KEY_LONG_TICKS */

/* 长按重复触发时间，即隔多长时间触发一次，换算成滴答数 */
#ifndef GM_MULTI_KEY_REPEAT_TRIG_TICKS
#define GM_MULTI_KEY_REPEAT_TRIG_TICKS  (100u / GM_MULTI_KEY_POLL_INTERVAL)
#endif  /* GM_MULTI_KEY_REPEAT_TRIG_TICKS */

/* 连击最大次数，主要是怕数据溢出 */
#ifndef GM_MULTI_KEY_CLICK_COUNT_MAX
#define GM_MULTI_KEY_CLICK_COUNT_MAX    GM_TYPE_UNSIGNED_MAX(GM_U16)
#endif  /* GM_MULTI_KEY_CLICK_COUNT_MAX */

/* 按键状态定义 */
typedef enum
{
    GM_MULTI_KEY_FSM_STATUS_RELEASE,        /* 按键已释放状态 */
    GM_MULTI_KEY_FSM_STATUS_PRESS,          /* 按键已按下状态 */
    GM_MULTI_KEY_FSM_STATUS_WAIT_PRESS_AGIN,/* 按键松开，等待再次按下，用于判断是否连击 */
    GM_MULTI_KEY_FSM_STATUS_PRESS_AGIN,     /* 按键再次按下状态，此时不能再响应长按，所以和GM_MULTI_KEY_FSM_STATUS_PRESS处理不同 */
    GM_MULTI_KEY_FSM_STATUS_WAIT_RELEASE,   /* 等待按键释放状态 */
    GM_MULTI_KEY_FSM_STATUS_NUM,
} GM_MULTI_KEY_FSM_STATUS;

/* 执行事件回调 */
#define GM_MULTI_KEY_EVENT_PROC(pkey)           \
    do                                          \
    {                                           \
        if (pkey->event_proc != GM_NULL)        \
        {                                       \
            pkey->event_proc(1, (char**)pkey);  \
        }                                       \
    } while (0)

/* 链表头节点 */
static GM_MULTI_KEY* gm_multi_key_list_head = GM_NULL;

/*******************************************************************************
** 函数名称：GM_MultiKey_Init
** 函数作用：初始化按键
** 输入参数：pkey - 按键指针对象
**           read_io_level - 按键读取IO电平函数指针
**           event_cb - 事件回调函数
** 输出参数：无
** 使用样例：GM_MultiKey_Init(&key1, key1_read);
** 函数备注：此函数会读取一次电平存放起来
*******************************************************************************/
void GM_MultiKey_Init(GM_MULTI_KEY* pkey, 
                      GM_MULTI_KEY_READ_IO read_io_level,
                      GM_CALLBACK event_cb)
{
    memset(pkey, 0, sizeof(GM_MULTI_KEY));

    pkey->event = GM_MULTI_KEY_EVENT_NONE;
    pkey->event_proc = event_cb;
    pkey->read_io_level = read_io_level;
    pkey->level = pkey->read_io_level();
}

/*******************************************************************************
** 函数名称：GM_MultiKey_Add
** 函数作用：添加一个按键进入链表中
** 输入参数：pkey - 按键指针对象
** 输出参数：是否添加成功
** 使用样例：GM_MultiKey_Add(&key1);
** 函数备注：pkey非法或已存在均会返回失败
*******************************************************************************/
GM_BOOL GM_MultiKey_Add(GM_MULTI_KEY* pkey)
{
    GM_MULTI_KEY* target;

    if (pkey == GM_NULL)
    {
        /* 指针非法 */
        return GM_FALSE;
    }

    /* 搜寻是否已存在于链表中 */
    target = gm_multi_key_list_head;
    while (target != GM_NULL)
    {
        if (target == pkey)
        {
            /* 已存在 */
            return GM_FALSE;
        }
        target = target->next;
    }
    /* 未搜索到，添加到头部 */
    pkey->next = gm_multi_key_list_head;
    /* 更新头节点 */
    gm_multi_key_list_head = pkey;

    return GM_TRUE;
}

/*******************************************************************************
** 函数名称：GM_MultiKey_Delete
** 函数作用：删除一个按键进入链表中
** 输入参数：pkey - 按键指针对象
** 输出参数：是否删除成功
** 使用样例：GM_MultiKey_Delete(&key1);
** 函数备注：pkey非法或不存在均会返回失败
*******************************************************************************/
GM_BOOL GM_MultiKey_Delete(GM_MULTI_KEY* pkey)
{
    GM_MULTI_KEY* ptemp;
    GM_MULTI_KEY* prev;

    if ((gm_multi_key_list_head == GM_NULL) || (pkey == GM_NULL))
    {
        /* 指针非法或链表空 */
        return GM_FALSE;
    }

    if (gm_multi_key_list_head == pkey)
    {
        /* 如果需要删除的节点位于链表头 */
        gm_multi_key_list_head = gm_multi_key_list_head->next;
        pkey->next = GM_NULL;
        return GM_TRUE;
    }

    /* 从第二个开始搜索 */
    prev = gm_multi_key_list_head;
    ptemp = gm_multi_key_list_head->next;
    for (; ptemp != GM_NULL; ptemp = ptemp->next)
    {
        if (ptemp == pkey)
        {
            /* 搜索到，删除节点 */
            prev->next = ptemp->next;
            ptemp->next = GM_NULL;
            return GM_TRUE;
        }
        prev = ptemp;
    }

    /* 未搜索到 */
    return GM_FALSE;
}

/*******************************************************************************
** 函数名称：GM_MultiKey_PollOneKey
** 函数作用：轮询一个按键
** 输入参数：pkey - 按键指针对象
** 输出参数：无
** 使用样例：GM_MultiKey_PollOneKey(&key1);
** 函数备注：pkey一定要是合法的，此函数为了效率不检查此数据指针合法性
*******************************************************************************/
static void GM_MultiKey_PollOneKey(GM_MULTI_KEY* pkey)
{
    /* 读取电平 */
    GM_MULTI_KEY_LEVEL key_level = pkey->read_io_level();

    /* 消抖 */
    if (key_level != pkey->level)
    {
        if (++(pkey->debounce_cnt) >= GM_MULTI_KEY_DEBOUNCE_TICKS)
        {
            pkey->level = key_level;
            pkey->debounce_cnt = 0;
        }
    }
    else
    {
        pkey->debounce_cnt = 0;
    }

    /* 默认按键无事件 */
    pkey->event = GM_MULTI_KEY_EVENT_NONE;

    /* 状态机 */
    switch (pkey->status)
    {
    case GM_MULTI_KEY_FSM_STATUS_RELEASE:
        if (pkey->level == GM_MULTI_KEY_LEVEL_PRESS)
        {
            /* 按键按下事件 */
            pkey->event = GM_MULTI_KEY_EVENT_PRESS;
            /* 清除计数器 */
            pkey->count = 0;
            /* 清除连击计数 */
            pkey->click_cnt = 1;
            /* 切换到按键按下状态 */
            pkey->status = GM_MULTI_KEY_FSM_STATUS_PRESS;
        }
        break;

    case GM_MULTI_KEY_FSM_STATUS_PRESS:
        if (pkey->level != GM_MULTI_KEY_LEVEL_PRESS)
        {
            /* 未达到长按时间之前抬起，按键松开事件 */
            pkey->event = GM_MULTI_KEY_EVENT_RELEASE;
            /* 清除计数器 */
            pkey->count = 0;
            /* 切换到等待连击状态 */
            pkey->status = GM_MULTI_KEY_FSM_STATUS_WAIT_PRESS_AGIN;
        }
        else if (++(pkey->count) > GM_MULTI_KEY_LONG_TICKS)
        {
            /* 超过长按时间还未松开，首次触发长按事件 */
            pkey->event = GM_MULTI_KEY_EVENT_LONG_FIRST;
            /* 切换到等待按键松开状态 */
            pkey->status = GM_MULTI_KEY_FSM_STATUS_WAIT_RELEASE;
        }
        break;

    case GM_MULTI_KEY_FSM_STATUS_WAIT_PRESS_AGIN:
        if (pkey->level == GM_MULTI_KEY_LEVEL_PRESS)
        {
            /* 按键按下事件 */
            pkey->event = GM_MULTI_KEY_EVENT_PRESS;
            /* 连击计数加1 */
            if (pkey->click_cnt < GM_MULTI_KEY_CLICK_COUNT_MAX)
            {
                pkey->click_cnt++;
            }
            /* 清除计数器 */
            pkey->count = 0;
            /* 切换到按键再次按下状态 */
            pkey->status = GM_MULTI_KEY_FSM_STATUS_PRESS_AGIN;
        }
        else if (++(pkey->count) > GM_MULTI_KEY_HIT_AGIN_TICKS)
        {
            /* 超过连击间隔时间还未再次按下按键 */
            if (pkey->click_cnt == 1) 
            {
                /* 仅点击了一次 */
                pkey->event = GM_MULTI_KEY_EVENT_SINGLE_CLICK;
            }
            else if (pkey->click_cnt == 2) 
            {
                /* 双击 */
                pkey->event = GM_MULTI_KEY_EVENT_DOUBLE_CLICK;
            }
            else
            {
                /* 多次连击 */
                pkey->event = GM_MULTI_KEY_EVENT_MULTI_CLICK;
            }
            /* 切换到按键抬起状态，此次按键流程结束 */
            pkey->status = GM_MULTI_KEY_FSM_STATUS_RELEASE;
        }
        break;

    case GM_MULTI_KEY_FSM_STATUS_PRESS_AGIN:
        if (pkey->level != GM_MULTI_KEY_LEVEL_PRESS)
        {
            /* 按键松开事件 */
            pkey->event = GM_MULTI_KEY_EVENT_RELEASE;
            /* 清除计数器 */
            pkey->count = 0;
            /* 切换到等待按键再次按下状态 */
            pkey->status = GM_MULTI_KEY_FSM_STATUS_WAIT_PRESS_AGIN;
        }
        break;

    case GM_MULTI_KEY_FSM_STATUS_WAIT_RELEASE:
        if (pkey->level == GM_MULTI_KEY_LEVEL_PRESS)
        {
            if (++(pkey->count) > GM_MULTI_KEY_REPEAT_TRIG_TICKS)
            {
                /* 清除计数 */
                pkey->count = 0;
                /* 继续按下，按键重复按下事件 */
                pkey->event = GM_MULTI_KEY_EVENT_REPEAT;
            }
        }
        else
        {
            /* 按键松开，按键抬起事件 */
            pkey->event = GM_MULTI_KEY_EVENT_RELEASE;
            /* 清除计数器 */
            pkey->count = 0;
            /* 清除连击计数 */
            pkey->click_cnt = 0;
            /* 回到释放状态 */
            pkey->status = GM_MULTI_KEY_FSM_STATUS_RELEASE;
        }
        break;

    default:
        /* 异常，回到释放状态 */
        pkey->count = 0;
        pkey->click_cnt = 0;
        pkey->status = GM_MULTI_KEY_FSM_STATUS_RELEASE;
        break;
    }

    /* 有事件产生，执行回调 */
    if (pkey->event != GM_MULTI_KEY_EVENT_NONE)
    {
        GM_MULTI_KEY_EVENT_PROC(pkey);
    }
}

/*******************************************************************************
** 函数名称：GM_MultiKey_PollEvent
** 函数作用：执行按键事件轮询
** 输入参数：无
** 输出参数：无
** 使用样例：GM_MultiKey_PollEvent();
** 函数备注：此函数会依次每个按键进行轮询，有事件执行事件回调函数
*******************************************************************************/
void GM_MultiKey_PollEvent(void)
{
    GM_MULTI_KEY* pkey;

    for (pkey = gm_multi_key_list_head; pkey != GM_NULL; pkey = pkey->next)
    {
        GM_MultiKey_PollOneKey(pkey);
    }
}
