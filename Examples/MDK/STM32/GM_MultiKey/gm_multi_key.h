/*******************************************************************************
** 文件名称：gm_multi_key_.h
** 文件作用：多功能按键
** 编写作者：Tom Free 付瑞彪
** 编写时间：20-05-27
** 文件备注：此文件是多功能按键的头文件，主要包括API声明和数据类型定义
**			 
** 更新记录：
**          2020-05-27 -> 创建文件                             <Tom Free 付瑞彪>
**
**              Copyright (c) 2018-2020 付瑞彪 All Rights Reserved
**
**       1 Tab == 4 Spaces     UTF-8     ANSI C Language(C99)
*******************************************************************************/
#ifndef __GM_MULTI_KEY__H__
#define __GM_MULTI_KEY__H__

#include "gm_type.h"

/* 按键电平定义 */
typedef enum
{
    GM_MULTI_KEY_LEVEL_PRESS,           /* 按下 */
    GM_MULTI_KEY_LEVEL_RELEASE,         /* 释放 */
} GM_MULTI_KEY_LEVEL;

/* 按键事件 */
typedef enum
{
    GM_MULTI_KEY_EVENT_NONE,            /* 无事件 */
    GM_MULTI_KEY_EVENT_PRESS,           /* 按下事件，按下边沿触发 */
    GM_MULTI_KEY_EVENT_RELEASE,         /* 释放事件，松开边沿触发 */
    GM_MULTI_KEY_EVENT_LONG_FIRST,      /* 首次触发长按，按下达到长按时间时触发 */
    GM_MULTI_KEY_EVENT_REPEAT,          /* 重复按下，按下时一直触发 */
    GM_MULTI_KEY_EVENT_SINGLE_CLICK,    /* 单击 */
    GM_MULTI_KEY_EVENT_DOUBLE_CLICK,    /* 双击 */
    GM_MULTI_KEY_EVENT_MULTI_CLICK,     /* 连击 */
    GM_MULTI_KEY_EVENT_NUM,
} GM_MULTI_KEY_EVENT;

/* 读取IO口电平 */
typedef GM_MULTI_KEY_LEVEL(*GM_MULTI_KEY_READ_IO)(void);

/* 按键结构定义 */
typedef struct GM_MULTI_KEY_NODE
{
    GM_U16 count;                       /* 按键计数器，用于时间计数，用户不可摆弄，只可读 */
    GM_U8 status;                       /* 按键状态机状态，用户不可摆弄，只可读 */
    GM_U8 debounce_cnt;                 /* 消抖计数，用户不可摆弄，只可读 */
    GM_U16 click_cnt;                   /* 连续点击计数，实现多连击，用户不可摆弄，只可读 */
    GM_MULTI_KEY_EVENT event;           /* 按键事件，用户不可摆弄，只可读 */
    GM_MULTI_KEY_LEVEL level;           /* IO电平，用户不可摆弄，只可读 */
    GM_MULTI_KEY_READ_IO read_io_level; /* IO读取函数，用户传入，需保证合法性，可读写 */
    GM_CALLBACK event_proc;             /* 事件处理回调，用户传入，需保证合法性，可读写 */
    struct GM_MULTI_KEY_NODE* next;     /* 链表指针，用户不可摆弄，只可读 */
} GM_MULTI_KEY;

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

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
                      GM_CALLBACK event_cb);

/*******************************************************************************
** 函数名称：GM_MultiKey_Add
** 函数作用：添加一个按键进入链表中
** 输入参数：pkey - 按键指针对象
** 输出参数：是否添加成功
** 使用样例：GM_MultiKey_Add(&key1);
** 函数备注：pkey非法或已存在均会返回失败
*******************************************************************************/
GM_BOOL GM_MultiKey_Add(GM_MULTI_KEY* pkey);

/*******************************************************************************
** 函数名称：GM_MultiKey_Delete
** 函数作用：删除一个按键进入链表中
** 输入参数：pkey - 按键指针对象
** 输出参数：是否删除成功
** 使用样例：GM_MultiKey_Delete(&key1);
** 函数备注：pkey非法或不存在均会返回失败
*******************************************************************************/
GM_BOOL GM_MultiKey_Delete(GM_MULTI_KEY* pkey);

/*******************************************************************************
** 函数名称：GM_MultiKey_PollEvent
** 函数作用：执行按键事件轮询
** 输入参数：无
** 输出参数：无
** 使用样例：GM_MultiKey_PollEvent();
** 函数备注：此函数会依次每个按键进行轮询，有事件执行事件回调函数
*******************************************************************************/
void GM_MultiKey_PollEvent(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* __GM_MULTI_KEY__H__ */
