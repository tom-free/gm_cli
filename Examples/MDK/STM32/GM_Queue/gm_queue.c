/*******************************************************************************
** 文件名称：queue.c
** 文件作用：队列操作源文件
** 编写作者：Tom Free 付瑞彪
** 编写时间：2018-11-06
** 文件备注：
**
**
** 更新记录：
**          2018-11-06 -> 创建文件                          <Tom Free 付瑞彪>
**
**
**       Copyright (c) 深圳市三派智能科技有限公司 All Rights Reserved
**
**       1 Tab == 4 Spaces     UTF-8     ANSI C Language(C99)
*******************************************************************************/

/*******************************************************************************
******************************** 队列结构 **************************************
*                       +----------------------------------+                   *
*                       |                                  |                   *
*               +---------------+                          |                   *
*               |               |                          |                   *
*               +---------------+                          |                   *
*               |               |                          |                   *
*               +---------------+                          |                   *
*               |               |                          |                   *
*               +---------------+                          |                   *
*               |               |<--wPointer(move)         |                   *
*               +---------------+                          |                   *
*               |  DataBlock n  |                          |                   *
*               +---------------+                          |                   *
*               .       .       .                          |                   *
*               .       .       .                          |                   *
*               .       .       .                          |                   *
*               +---------------+                          |                   *
*               |  DataBlock 3  |                          |                   *
*               +---------------+                          |                   *
*               |  DataBlock 2  |                          |                   *
*               +---------------+                          |                   *
*               |  DataBlock 1  |<--rPointer(move)         |                   *
*               +---------------+                          |                   *
*               .       .       .                          |                   *
*               .       .       .                          |                   *
*               .       .       .                          |                   *
*               +---------------+                          |                   *
*                       ^                                  |                   *
*                      /|\                                 |                   *
*                       +----------------------------------+                   *
*******************************************************************************/

/* 队列头文件 */
#include "gm_queue.h"

/* 内存管理头文件 */
#include "../GM_Memory/gm_mem.h"

/* 临界段处理方法定义 */
#define GM_QUEUE_ENTER_CRITICAL()   __disable_irq()
#define GM_QUEUE_EXIT_CRITICAL()    __enable_irq()

/* 队列结构定义 */
typedef struct
{
    GM_SIZE  wPointer;  /* 写指针 */
    GM_SIZE  rPointer;  /* 读指针 */
    GM_SIZE  Length;    /* 数据数量 */
    GM_SIZE  Capacity;  /* 队列缓存区大小 */
    GM_SIZE  DBSize;    /* 数据块大小 */
    void*    qBuf;      /* 队列缓存区首地址 */
} GM_QUEUE_OBJ;

/*******************************************************************************
** 函数名称：GM_Queue_Create
** 函数作用：创建一个队列
** 输入参数：Capacity - 容量
**           DBSize   - 数据单元大小，即存放的单个数据所占字节数
** 输出参数：队列指针
** 使用范例：GM_QUEUE queue = GM_Queue_Create(10, sizeof(float));
** 函数备注：
*******************************************************************************/
GM_QUEUE GM_Queue_Create(GM_SIZE Capacity, GM_SIZE DBSize)
{
    GM_QUEUE_OBJ* pQueue = GM_NULL;
    void*         pQueueBuf = GM_NULL;

    pQueue = (GM_QUEUE_OBJ*)GM_Mem_Alloc(sizeof(GM_QUEUE_OBJ));
    if (GM_NULL == pQueue)
    {
        /* 如果无法创建管理器，返回创建失败 */
        return GM_NULL;
    }

    /* 创建堆栈存储区 */
    pQueueBuf = GM_Mem_Alloc(Capacity * DBSize);
    if (GM_NULL == pQueueBuf)
    {
        /* 如果无法创建堆队列空间，释放掉管理器，并返回创建失败 */
        GM_Mem_Free(pQueue);
        return GM_NULL;
    }

    pQueue->wPointer = 0;
    pQueue->rPointer = 0;
    pQueue->Capacity = Capacity;
    pQueue->DBSize = DBSize;
    pQueue->Length = 0;
    pQueue->qBuf = pQueueBuf;

    return (GM_QUEUE)pQueue;
}

/*******************************************************************************
** 函数名称：GM_Queue_Delete
** 函数作用：删除队列
** 输入参数：queue - 队列句柄
** 输出参数：是否删除成功
** 使用范例：GM_BOOL res = GM_Queue_Delete(queue);
** 函数备注：
*******************************************************************************/
GM_BOOL GM_Queue_Delete(GM_QUEUE queue)
{
    /* 句柄转obj */
    GM_QUEUE_OBJ* pQueue = (GM_QUEUE_OBJ*)queue;

    if (GM_NULL == pQueue)
    {
        /* 无效队列 */
        return GM_FALSE;
    }

    /* 先释放存储区 */
    if (GM_NULL != pQueue->qBuf)
    {
        GM_Mem_Free(pQueue->qBuf);
    }

    /* 释放管理器 */
    GM_Mem_Free(pQueue);

    return GM_TRUE;
}

/*******************************************************************************
** 函数名称：GM_Queue_Clear
** 函数作用：队列清空
** 输入参数：queue - 队列句柄
** 输出参数：无
** 使用范例：GM_Queue_Clear(queue);
** 函数备注：必须保证queue指针有效
*******************************************************************************/
GM_BOOL GM_Queue_Clear(GM_QUEUE queue)
{
    /* 句柄转obj */
    GM_QUEUE_OBJ* pQueue = (GM_QUEUE_OBJ*)queue;

    if ((GM_NULL == pQueue) ||
        (GM_NULL == pQueue->qBuf))
    {
        /* 无效队列或无效数据区 */
        return GM_FALSE;
    }

    /* 进入临界区 */
	GM_QUEUE_ENTER_CRITICAL();

    pQueue->wPointer = 0;   /* 写指针   */
    pQueue->rPointer = 0;   /* 读指针   */
    pQueue->Length = 0;     /* 数据个数 */

    /* 退出临界区 */
	GM_QUEUE_EXIT_CRITICAL();

    return GM_TRUE;
}

/*******************************************************************************
** 函数名称：GM_Queue_Read
** 函数作用：从队列中读取一个数据
** 输入参数：queue - 队列句柄
**           pdata - 读出数据存放地址
** 输出参数：操作结果，GM_TRUE - 成功，GM_FALSE - 失败
** 使用范例：GM_BOOL res = GM_Queue_Read(queue, &data);
** 函数备注：必须保证queue和pdata指针有效
*******************************************************************************/
GM_BOOL GM_Queue_Read(GM_QUEUE queue, void* pdata)
{
    GM_BOOL res = GM_FALSE;
    /* 句柄转obj */
    GM_QUEUE_OBJ* pQueue = (GM_QUEUE_OBJ*)queue;

    if ((GM_NULL == pQueue) ||
        (GM_NULL == pQueue->qBuf))
    {
        /* 无效队列或无效数据区 */
        return GM_FALSE;
    }

	/* 进入临界区 */
	GM_QUEUE_ENTER_CRITICAL();

    do
    {
        /* 判断队列是否为空，为空读取数据失败 */
        if (pQueue->Length == 0)
        {
            res = GM_FALSE;
            break;
        }

        /* 取出数据 */
        GM_Mem_Copy(pdata, (GM_U8*)pQueue->qBuf + pQueue->rPointer * pQueue->DBSize, pQueue->DBSize);
        pQueue->Length--;

        /* 刷新读指针 */
        if (++pQueue->rPointer == pQueue->Capacity)
        {
            pQueue->rPointer = 0;
        }

        res = GM_TRUE;

    } while (0);

	/* 退出临界区 */
	GM_QUEUE_EXIT_CRITICAL();

    return res;
}

/*******************************************************************************
** 函数名称：GM_Queue_Write
** 函数作用：向队列中写入一个数据
** 输入参数：queue - 队列句柄
**           pdata - 待写入数据地址
** 输出参数：操作结果，GM_TRUE - 成功，GM_FALSE - 失败
** 使用范例：GM_BOOL res = GM_Queue_Write(queue, &data);
** 函数备注：必须保证queue和pdata指针有效
*******************************************************************************/
GM_BOOL GM_Queue_Write(GM_QUEUE queue, void* pdata)
{
    GM_BOOL res = GM_FALSE;
    /* 句柄转obj */
    GM_QUEUE_OBJ* pQueue = (GM_QUEUE_OBJ*)queue;

    if ((GM_NULL == pQueue) ||
        (GM_NULL == pQueue->qBuf))
    {
        /* 无效队列或无效数据区 */
        return GM_FALSE;
    }

  	/* 进入临界区 */
	GM_QUEUE_ENTER_CRITICAL();

    do
    {
        /* 判断队列是否为满，为满写入数据失败 */
        if (pQueue->Length == pQueue->Capacity)
        {
            res = GM_FALSE;
            break;
        }

        /* 写数据到队列 */
        GM_Mem_Copy((GM_U8*)pQueue->qBuf + pQueue->wPointer * pQueue->DBSize, pdata, pQueue->DBSize);
        pQueue->Length++;

        /* 刷新写指针 */
        if (++pQueue->wPointer == pQueue->Capacity)
        {
            pQueue->wPointer = 0;
        }

        res = GM_TRUE;

    } while (0);

    /* 退出临界区 */
	GM_QUEUE_EXIT_CRITICAL();

    return res;
}

/*******************************************************************************
** 函数名称：GM_Queue_Length
** 函数作用：获得队列中数据长度
** 输入参数：queue - 队列句柄
** 输出参数：数据长度
** 使用范例：GM_U8 len = GM_Queue_Length(queue);
** 函数备注：必须保证queue指针有效
*******************************************************************************/
GM_SIZE GM_Queue_Length(GM_QUEUE queue)
{
    /* 句柄转obj */
    GM_QUEUE_OBJ* pQueue = (GM_QUEUE_OBJ*)queue;

    return pQueue->Length;
}
