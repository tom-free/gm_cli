/*******************************************************************************
** 文件名称：heap2.C
** 文件作用：动态内存管理策略2源文件
** 编写作者：TomFree（付瑞彪）
** 编写时间：2018-04-20
** 更新时间：2018-04-20
** 文件备注：
**         本内存管理是FreeRTOS的脱离版
** 更新记录：
**         2018-04-20 -> 添加注释
*******************************************************************************/

#include "heap.h"

/* 如果选择的是第二种内存管理策略 */
#if (heapALLOCATION_TYPE == 2)

/* 包含类型定义头文件 */
#include "../GM_Type/gm_type.h"

/* 由于进行对齐时会出现最多heapALIGNMENT_BYTE的浪费,所以进行实际动态内存字节数校正 */
#define heapREAL_HEAP_SIZE		(heapTOTAL_HEAP_SIZE - heapALIGNMENT_BYTE)

/* 为堆分配内存 */
#if( heapUSER_ALLOCATED_HEAP == 1 )
	/* 如果用户为了将动态数据放在特殊的区域或地址,用户可以自己进行外部定义 */
	extern uint8_t ucHeap[ heapTOTAL_HEAP_SIZE ];
#else
	/* 否则系统自己定义动态数据区 */
	static uint8_t ucHeap[ heapTOTAL_HEAP_SIZE ];
#endif /* heapUSER_ALLOCATED_HEAP == 1 */

/* 此结构为空闲块链表结构,用于连接空闲块 */
typedef struct BLOCK_LINK
{
	struct BLOCK_LINK *pxNext;
	size_t			   xSize;
}BlockLink_t;

/* 定义对齐后的结构体大小 */
#define heapSTRUCT_SIZE			((sizeof(BlockLink_t) + (heapALIGNMENT_BYTE - 1)) & ~heapALIGNMENT_MASK)

/* 定义最小的块大小 */
#define heapMINIMUM_BLOCK_SIZE	((size_t)(heapSTRUCT_SIZE * 2))

/* 这两个结构用于遍历空闲链表 */
static BlockLink_t xStart, xEnd;

/* 此变量保存剩余空间大小 */
static size_t xFreeBytesRemaining = heapREAL_HEAP_SIZE;

/******************************************************
* 函 数 名：prvInsertFreeBlockIntoFreeList
* 功    能：将空闲块插入到空闲链表中
* 传入参数：pxFreeBlock -待插入的空闲块
* 传出参数：无
* 使用举例：prvInsertFreeBlockIntoFreeList(pxBlock1);
*******************************************************/
static void prvInsertFreeBlockIntoFreeList(BlockLink_t* pxFreeBlock)
{
	BlockLink_t* pxTemp;
	size_t		 xSize;

	xSize = pxFreeBlock->xSize;

	pxTemp = &xStart;
	while(pxTemp->pxNext->xSize < xSize)
	{
		pxTemp = pxTemp->pxNext;
	}
	pxFreeBlock->pxNext = pxTemp->pxNext;
	pxTemp->pxNext = pxFreeBlock;
}

/******************************************************
* 函 数 名：prvHeapInit
* 功    能：初始化堆
* 传入参数：无
* 传出参数：无
* 使用举例：prvHeapInit();
*******************************************************/
static void prvHeapInit(void)
{
	BlockLink_t *pxFirstFreeBlock;
	uint8_t *pucAlignedHeap;

	pucAlignedHeap = (uint8_t *)(((typePOINTER_SIZE ) &ucHeap[heapALIGNMENT_BYTE]) & (~((typePOINTER_SIZE)heapALIGNMENT_MASK)));

	xStart.pxNext = (BlockLink_t *)pucAlignedHeap;
	xStart.xSize  = (size_t)0;

	xEnd.pxNext = NULL;
	xEnd.xSize  = heapREAL_HEAP_SIZE;
	
	pxFirstFreeBlock = (BlockLink_t *)pucAlignedHeap;
	pxFirstFreeBlock->xSize = heapREAL_HEAP_SIZE;
	pxFirstFreeBlock->pxNext = &xEnd;
}

/******************************************************
* 函 数 名：pvHeapMalloc
* 功    能：动态内存申请
* 传入参数：xWantedSize -需要的内存大小
* 传出参数：void * 型的地址
* 使用举例：p = pvHeapMalloc(100);
*******************************************************/
void* pvHeapMalloc(size_t xWantedSize)
{
	BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
	static uint8 xInitFlag = 0;
	void *pvReturn = NULL;

	/* 进入临界区 */
	heapINTO_CRITICAL();

	/* 如果没有进行初始化 */
	if(xInitFlag == 0)
	{
		/* 初始化堆内存 */
		prvHeapInit();
		/* 置位初始化标志 */
		xInitFlag = 1;
	}

	/* 如果申请字节数有效 */
	if(xWantedSize > 0)
	{
		/* 得到总字节数,加上了结构体 */
		xWantedSize += heapSTRUCT_SIZE;
		/* 如果字节数没有字节对齐 */
		if((xWantedSize & heapALIGNMENT_MASK) != 0)
		{
			/* 进行字节对齐 */
			xWantedSize += (heapALIGNMENT_BYTE - (xWantedSize & heapALIGNMENT_MASK));
		}
	}

	/* 如果实际申请字节数合法 */
	if((xWantedSize > 0) && (xWantedSize < heapREAL_HEAP_SIZE))
	{
		pxPreviousBlock = &xStart;
		pxBlock = xStart.pxNext;
		/* 进行遍历,寻找合适的空间 */
		while((pxBlock->xSize < xWantedSize) && (pxBlock->pxNext != NULL))
		{
			pxPreviousBlock = pxBlock;
			pxBlock = pxBlock->pxNext;
		}

		/* 如果找到了合适内存空间,否则直接退出 */
		if(pxBlock != &xEnd)
		{
			/* 将找到的内存块地址返回 */
			pvReturn = (void*)(((uint8_t*)pxBlock) + heapSTRUCT_SIZE );
			
			/* 将该内存块移除空闲链表 */
			pxPreviousBlock->pxNext = pxBlock->pxNext;

			/* 如果该内存块剩下的内存大于最小的内存长度,需要分块,否则不许分块 */
			if( ( pxBlock->xSize - xWantedSize ) > heapMINIMUM_BLOCK_SIZE )
			{
				/* 计算得到剩下的内存起始地址,将剩下的内存作为一个新内存块 */
				pxNewBlockLink = (BlockLink_t*) (((uint8_t*)pxBlock) + xWantedSize);

				/* 得到新内存块大小 */
				pxNewBlockLink->xSize = pxBlock->xSize - xWantedSize;

				/* 更新已分配的内存大小 */
				pxBlock->xSize = xWantedSize;

				/* 将新内存块插入空闲链表 */
				prvInsertFreeBlockIntoFreeList( ( pxNewBlockLink ) );
			}
			/* 更新剩余内存字节数 */
			xFreeBytesRemaining -= pxBlock->xSize;
		}
	}

	/* 退出临界区 */
	heapEXIT_CRITICAL();

	/* 返回申请到的地址 */
	return pvReturn;
}

/******************************************************
* 函 数 名：vHeapFree
* 功    能：动态内存释放
* 传入参数：pv -待释放的内存首地址
* 传出参数：无
* 使用举例：vHeapFree(p);
*******************************************************/
void vHeapFree(void *pv)
{
	uint8_t *puc = (uint8_t*)pv;
	BlockLink_t *pxLink;

	/* 如果pv合法 */
	if( pv != NULL )
	{
		/* 计算实际的起始地址,需要减去结构体大小 */
		puc -= heapSTRUCT_SIZE;
		/* 得到该释放的内存块结构体*/
		pxLink = ( BlockLink_t * ) puc;

		/* 进入临界区 */
		heapINTO_CRITICAL();

		/* 将释放后的内存块插入空闲链表 */
		prvInsertFreeBlockIntoFreeList(pxLink);
		/* 更新空闲内存字节数 */
		xFreeBytesRemaining += pxLink->xSize;

		/* 退出临界区 */
		heapEXIT_CRITICAL();
	}
}

/******************************************************
* 函 数 名：xHeapGetFreeSize
* 功    能：获取空闲内存大小
* 传入参数：无
* 传出参数：空闲内存大小
* 使用举例：num = xHeapGetFreeSize();
*******************************************************/
size_t xHeapGetFreeSize( void )
{
	/* 返回剩余空间值 */
	return xFreeBytesRemaining;
}

#endif  /* heapALLOCATION_TYPE == 2 */
