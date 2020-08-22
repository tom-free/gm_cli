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

/* 队列头文件 */
#include "queue.h"

/* 临界段处理方法定义 */
#define QUEUE_ENTER_CRITICAL()    disableInterrupts()  /* 失能中断 */
#define QUEUE_EXIT_CRITICAL()     enableInterrupts()   /* 使能中断 */

/*******************************************************************************
** 函数名称：Queue_Init
** 函数作用：初始化队列
** 输入参数：queue - 队列指针，buf - 队列缓冲区地址，size - 缓冲区大小
** 输出参数：无
** 使用范例：Queue_Init(&xQueue, buf, 32);
** 函数备注：必须保证queue和buf指针有效
*******************************************************************************/
void Queue_Init(QUEUE *queue, GM_U8 *buf, GM_U8 size)
{
    /* 进入临界区 */
	QUEUE_ENTER_CRITICAL();

	queue->wptr = 0;        /* 写指针     */
	queue->rptr = 0;        /* 读指针     */
    queue->count = 0;       /* 数据个数   */
	queue->buf  = buf;      /* 缓冲区指针 */
	queue->size = size;     /* 缓冲区大小 */

    /* 退出临界区 */
	QUEUE_EXIT_CRITICAL();
}

/*******************************************************************************
** 函数名称：Queue_Clear
** 函数作用：队列清空
** 输入参数：queue - 队列指针
** 输出参数：无
** 使用范例：Queue_Clear(&xQueue);
** 函数备注：必须保证queue指针有效
*******************************************************************************/
void Queue_Clear(QUEUE *queue)
{
    /* 进入临界区 */
	QUEUE_ENTER_CRITICAL();

    queue->wptr = 0;        /* 写指针   */
	queue->rptr = 0;        /* 读指针   */
    queue->count = 0;       /* 数据个数 */

    /* 退出临界区 */
	QUEUE_EXIT_CRITICAL();
}

/*******************************************************************************
** 函数名称：Queue_Read
** 函数作用：从队列中读取一个数据
** 输入参数：queue - 队列指针，pdata - 数据地址
** 输出参数：操作结果，GM_TRUE - 成功，GM_FALSE - 失败
** 使用范例：GM_BOOL res = Queue_Read(&xQueue, &data);
** 函数备注：必须保证queue和pdata指针有效
*******************************************************************************/
GM_BOOL Queue_Read(QUEUE *queue, GM_U8* pdata)
{
    GM_BOOL res = GM_FALSE;

	/* 进入临界区 */
	QUEUE_ENTER_CRITICAL();

    do
    {
        /* 判断队列是否为空，为空读取数据失败 */
        if (queue->count == 0)
        {
            res = GM_FALSE;
            break;
        }

        /* 取出数据 */
        *pdata = queue->buf[queue->rptr];
        queue->count--;

        /* 刷新读指针 */
        if (++queue->rptr == queue->size)
        {
            queue->rptr = 0;
        }

        res = GM_TRUE;

    } while (0);

	/* 退出临界区 */
	QUEUE_EXIT_CRITICAL();

    return res;
}

/*******************************************************************************
** 函数名称：Queue_Write
** 函数作用：向队列中写入一个数据
** 输入参数：queue - 队列指针，data - 待写入数据
** 输出参数：操作结果，GM_TRUE - 成功，GM_FALSE - 失败
** 使用范例：GM_BOOL res = Queue_Write(&xQueue, 0x88);
** 函数备注：必须保证queue指针有效
*******************************************************************************/
GM_BOOL Queue_Write(QUEUE *queue, GM_U8 data)
{
    GM_BOOL res = GM_FALSE;

  	/* 进入临界区 */
	QUEUE_ENTER_CRITICAL();

    do
    {
        /* 判断队列是否为满，为满写入数据失败 */
        if (queue->count == queue->size)
        {
            res = GM_FALSE;
            break;
        }

        /* 写数据到队列 */
        queue->buf[queue->wptr] = data;
        queue->count++;

        /* 刷新写指针 */
        if (++queue->wptr == queue->size)
        {
            queue->wptr = 0;
        }

        res = GM_TRUE;

    } while (0);

    /* 退出临界区 */
	QUEUE_EXIT_CRITICAL();

    return res;
}

/*******************************************************************************
** 函数名称：Queue_Empty
** 函数作用：检查队列是否为空
** 输入参数：queue - 队列指针
** 输出参数：GM_TRUE - 空，无数据，GM_FALSE - 非空，有数据
** 使用范例：GM_BOOL res = Queue_Empty(&xQueue);
** 函数备注：必须保证queue指针有效
*******************************************************************************/
GM_BOOL Queue_Empty(QUEUE* queue)
{
    return queue->count ? GM_FALSE : GM_TRUE;
}

/*******************************************************************************
** 函数名称：Queue_NonEmpty
** 函数作用：检查是否有数据，即非空
** 输入参数：queue - 队列指针
** 输出参数：GM_TRUE - 非空，有数据，GM_FALSE - 空，无数据
** 使用范例：GM_BOOL res = Queue_NonEmpty(&xQueue);
** 函数备注：必须保证queue指针有效
*******************************************************************************/
GM_BOOL Queue_NonEmpty(QUEUE* queue)
{
    return queue->count ? GM_TRUE : GM_FALSE;
}

/*******************************************************************************
** 函数名称：Queue_GetLength
** 函数作用：获得队列中数据长度
** 输入参数：queue - 队列指针
** 输出参数：数据长度
** 使用范例：GM_U8 len = Queue_GetLength(&xQueue);
** 函数备注：必须保证queue指针有效
*******************************************************************************/
GM_U8 Queue_GetLength(QUEUE* queue)
{
    return queue->count;
}

/*******************************************************************************
** 函数名称：Queue_CancelLastWrite
** 函数作用：取消队列最后一次写入操作
** 输入参数：queue - 队列指针
** 输出参数：操作结果，GM_TRUE - 成功，GM_FALSE - 失败
** 使用范例：Queue_CancelLastWrite(&xQueue);
** 函数备注：必须保证queue指针有效
*******************************************************************************/
GM_BOOL Queue_CancelLastWrite(QUEUE *queue)
{
    GM_BOOL res = GM_FALSE;

  	/* 进入临界区 */
	QUEUE_ENTER_CRITICAL();

    /* 如果存在数据才可取消，否则无法取消，因为已被读走，取消已无意义 */
    if (queue->count)
    {
        if(queue->wptr == 0)
        {
            queue->wptr = queue->size - 1;
        }
        else
        {
            queue->wptr--;
        }

        res = GM_TRUE;
    }

	/* 退出临界区 */
	QUEUE_EXIT_CRITICAL();

	return res;
}

