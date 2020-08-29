/*******************************************************************************
** �ļ����ƣ�queue.c
** �ļ����ã����в���Դ�ļ�
** ��д���ߣ�Tom Free �����
** ��дʱ�䣺2018-11-06
** �ļ���ע��
**
**
** ���¼�¼��
**          2018-11-06 -> �����ļ�                          <Tom Free �����>
**
**
**       Copyright (c) �������������ܿƼ����޹�˾ All Rights Reserved
**
**       1 Tab == 4 Spaces     UTF-8     ANSI C Language(C99)
*******************************************************************************/

/*******************************************************************************
******************************** ���нṹ **************************************
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

/* ����ͷ�ļ� */
#include "gm_queue.h"

/* �ڴ����ͷ�ļ� */
#include "../GM_Memory/gm_mem.h"

/* �ٽ�δ��������� */
#define GM_QUEUE_ENTER_CRITICAL()   __disable_irq()
#define GM_QUEUE_EXIT_CRITICAL()    __enable_irq()

/* ���нṹ���� */
typedef struct
{
    GM_SIZE  wPointer;  /* дָ�� */
    GM_SIZE  rPointer;  /* ��ָ�� */
    GM_SIZE  Length;    /* �������� */
    GM_SIZE  Capacity;  /* ���л�������С */
    GM_SIZE  DBSize;    /* ���ݿ��С */
    void*    qBuf;      /* ���л������׵�ַ */
} GM_QUEUE_OBJ;

/*******************************************************************************
** �������ƣ�GM_Queue_Create
** �������ã�����һ������
** ���������Capacity - ����
**           DBSize   - ���ݵ�Ԫ��С������ŵĵ���������ռ�ֽ���
** �������������ָ��
** ʹ�÷�����GM_QUEUE queue = GM_Queue_Create(10, sizeof(float));
** ������ע��
*******************************************************************************/
GM_QUEUE GM_Queue_Create(GM_SIZE Capacity, GM_SIZE DBSize)
{
    GM_QUEUE_OBJ* pQueue = GM_NULL;
    void*         pQueueBuf = GM_NULL;

    pQueue = (GM_QUEUE_OBJ*)GM_Mem_Alloc(sizeof(GM_QUEUE_OBJ));
    if (GM_NULL == pQueue)
    {
        /* ����޷����������������ش���ʧ�� */
        return GM_NULL;
    }

    /* ������ջ�洢�� */
    pQueueBuf = GM_Mem_Alloc(Capacity * DBSize);
    if (GM_NULL == pQueueBuf)
    {
        /* ����޷������Ѷ��пռ䣬�ͷŵ��������������ش���ʧ�� */
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
** �������ƣ�GM_Queue_Delete
** �������ã�ɾ������
** ���������queue - ���о��
** ����������Ƿ�ɾ���ɹ�
** ʹ�÷�����GM_BOOL res = GM_Queue_Delete(queue);
** ������ע��
*******************************************************************************/
GM_BOOL GM_Queue_Delete(GM_QUEUE queue)
{
    /* ���תobj */
    GM_QUEUE_OBJ* pQueue = (GM_QUEUE_OBJ*)queue;

    if (GM_NULL == pQueue)
    {
        /* ��Ч���� */
        return GM_FALSE;
    }

    /* ���ͷŴ洢�� */
    if (GM_NULL != pQueue->qBuf)
    {
        GM_Mem_Free(pQueue->qBuf);
    }

    /* �ͷŹ����� */
    GM_Mem_Free(pQueue);

    return GM_TRUE;
}

/*******************************************************************************
** �������ƣ�GM_Queue_Clear
** �������ã��������
** ���������queue - ���о��
** �����������
** ʹ�÷�����GM_Queue_Clear(queue);
** ������ע�����뱣֤queueָ����Ч
*******************************************************************************/
GM_BOOL GM_Queue_Clear(GM_QUEUE queue)
{
    /* ���תobj */
    GM_QUEUE_OBJ* pQueue = (GM_QUEUE_OBJ*)queue;

    if ((GM_NULL == pQueue) ||
        (GM_NULL == pQueue->qBuf))
    {
        /* ��Ч���л���Ч������ */
        return GM_FALSE;
    }

    /* �����ٽ��� */
	GM_QUEUE_ENTER_CRITICAL();

    pQueue->wPointer = 0;   /* дָ��   */
    pQueue->rPointer = 0;   /* ��ָ��   */
    pQueue->Length = 0;     /* ���ݸ��� */

    /* �˳��ٽ��� */
	GM_QUEUE_EXIT_CRITICAL();

    return GM_TRUE;
}

/*******************************************************************************
** �������ƣ�GM_Queue_Read
** �������ã��Ӷ����ж�ȡһ������
** ���������queue - ���о��
**           pdata - �������ݴ�ŵ�ַ
** ������������������GM_TRUE - �ɹ���GM_FALSE - ʧ��
** ʹ�÷�����GM_BOOL res = GM_Queue_Read(queue, &data);
** ������ע�����뱣֤queue��pdataָ����Ч
*******************************************************************************/
GM_BOOL GM_Queue_Read(GM_QUEUE queue, void* pdata)
{
    GM_BOOL res = GM_FALSE;
    /* ���תobj */
    GM_QUEUE_OBJ* pQueue = (GM_QUEUE_OBJ*)queue;

    if ((GM_NULL == pQueue) ||
        (GM_NULL == pQueue->qBuf))
    {
        /* ��Ч���л���Ч������ */
        return GM_FALSE;
    }

	/* �����ٽ��� */
	GM_QUEUE_ENTER_CRITICAL();

    do
    {
        /* �ж϶����Ƿ�Ϊ�գ�Ϊ�ն�ȡ����ʧ�� */
        if (pQueue->Length == 0)
        {
            res = GM_FALSE;
            break;
        }

        /* ȡ������ */
        GM_Mem_Copy(pdata, (GM_U8*)pQueue->qBuf + pQueue->rPointer * pQueue->DBSize, pQueue->DBSize);
        pQueue->Length--;

        /* ˢ�¶�ָ�� */
        if (++pQueue->rPointer == pQueue->Capacity)
        {
            pQueue->rPointer = 0;
        }

        res = GM_TRUE;

    } while (0);

	/* �˳��ٽ��� */
	GM_QUEUE_EXIT_CRITICAL();

    return res;
}

/*******************************************************************************
** �������ƣ�GM_Queue_Write
** �������ã��������д��һ������
** ���������queue - ���о��
**           pdata - ��д�����ݵ�ַ
** ������������������GM_TRUE - �ɹ���GM_FALSE - ʧ��
** ʹ�÷�����GM_BOOL res = GM_Queue_Write(queue, &data);
** ������ע�����뱣֤queue��pdataָ����Ч
*******************************************************************************/
GM_BOOL GM_Queue_Write(GM_QUEUE queue, void* pdata)
{
    GM_BOOL res = GM_FALSE;
    /* ���תobj */
    GM_QUEUE_OBJ* pQueue = (GM_QUEUE_OBJ*)queue;

    if ((GM_NULL == pQueue) ||
        (GM_NULL == pQueue->qBuf))
    {
        /* ��Ч���л���Ч������ */
        return GM_FALSE;
    }

  	/* �����ٽ��� */
	GM_QUEUE_ENTER_CRITICAL();

    do
    {
        /* �ж϶����Ƿ�Ϊ����Ϊ��д������ʧ�� */
        if (pQueue->Length == pQueue->Capacity)
        {
            res = GM_FALSE;
            break;
        }

        /* д���ݵ����� */
        GM_Mem_Copy((GM_U8*)pQueue->qBuf + pQueue->wPointer * pQueue->DBSize, pdata, pQueue->DBSize);
        pQueue->Length++;

        /* ˢ��дָ�� */
        if (++pQueue->wPointer == pQueue->Capacity)
        {
            pQueue->wPointer = 0;
        }

        res = GM_TRUE;

    } while (0);

    /* �˳��ٽ��� */
	GM_QUEUE_EXIT_CRITICAL();

    return res;
}

/*******************************************************************************
** �������ƣ�GM_Queue_Length
** �������ã���ö��������ݳ���
** ���������queue - ���о��
** ������������ݳ���
** ʹ�÷�����GM_U8 len = GM_Queue_Length(queue);
** ������ע�����뱣֤queueָ����Ч
*******************************************************************************/
GM_SIZE GM_Queue_Length(GM_QUEUE queue)
{
    /* ���תobj */
    GM_QUEUE_OBJ* pQueue = (GM_QUEUE_OBJ*)queue;

    return pQueue->Length;
}
