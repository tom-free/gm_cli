/*******************************************************************************
** 文件名称：heap4.C
** 文件作用：动态内存管理策略4源文件
** 编写作者：TomFree（付瑞彪）
** 编写时间：2018-04-20
** 更新时间：2018-04-20
** 文件备注：
**         本内存管理是FreeRTOS的脱离版
** 更新记录：
**         2018-04-20 -> 添加注释
**         2018-05-27 -> 添加注释，整理代码规范
*******************************************************************************/
#include "heap.h"

#if (heapALLOCATION_TYPE == 4)

#include "../GM_Type/gm_type.h"

/* 定义对齐后的结构体大小 */
#define heapSTRUCT_SIZE			((sizeof(BlockLink_t) + (heapALIGNMENT_BYTE - 1)) & ~heapALIGNMENT_MASK)

/* 定义最小的块大小 */
#define heapMINIMUM_BLOCK_SIZE	((size_t)(heapSTRUCT_SIZE * 2))

/* 一个字节的位数 */
#define heapBITS_PER_BYTE		( ( size_t ) 8 )

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
	struct BLOCK_LINK *pxNext;  /* 下一个空闲块地址 */
    size_t			   xSize;   /* 此空闲块大小 */
}BlockLink_t;

/* 定义链表起始节点和链表终点节点指针 */
static BlockLink_t xStart, *pxEnd = NULL;

/* 剩余空闲字节数 */
static size_t xFreeBytesRemaining = 0U;

/* 历史最小空闲字节数 */
static size_t xMinimumEverFreeBytesRemaining = 0U;

/* 此块是否被分配标志，使用块的大小最高位来标识 */
static size_t xBlockAllocatedBit = 0;


/******************************************************************************
* 函 数 名：prvHeapInit
* 功    能：内存管理初始化
* 传入参数：无
* 传出参数：无
* 使用举例：prvHeapInit();
*******************************************************************************/
static void prvHeapInit(void)
{
    BlockLink_t *pxFirstFreeBlock;                  /* 第一个空闲块指针 */
    uint8_t *pucAlignedHeap;                        /* 对齐后的堆地址 */
    size_t uxAddress;                               /* 临时存放地址值 */
    size_t xTotalHeapSize = heapTOTAL_HEAP_SIZE;    /* 总堆大小 */

    /* 临时保存堆地址 */
    uxAddress = (size_t)ucHeap;
    /* 如果堆地址没对齐 */
    if ((uxAddress & heapALIGNMENT_MASK) != 0)
    {
        /* 进行对齐操作 */
        uxAddress += (heapALIGNMENT_BYTE - 1);
        uxAddress &= ~((size_t)heapALIGNMENT_MASK);
        /* 更正为对齐后的总堆大小 */
        xTotalHeapSize -= uxAddress - (size_t)ucHeap;
    }

    /* 对齐后的堆地址 */
    pucAlignedHeap = (uint8_t *)uxAddress;

    /* 将空闲链表开始节点的下一个节点指向对齐后的堆地址 */
    xStart.pxNext = (void *)pucAlignedHeap;
    /* 开始节点不保存任何其他信息，仅用来指示空闲块链表头，所以大小为0 */
    xStart.xSize = (size_t)0;

    /* 得到堆的尾部地址用于存放空闲链表结束块 */
    uxAddress = ((size_t)pucAlignedHeap) + xTotalHeapSize;
    uxAddress -= heapSTRUCT_SIZE;
    uxAddress &= ~((size_t)heapALIGNMENT_MASK);
    /* 结束块指针指向此地址 */
    pxEnd = (void *)uxAddress;
    /* 结束块大小为0，仅用来表示空闲块链表结束 */
    pxEnd->xSize = 0;
    /* 无下一个节点 */
    pxEnd->pxNext = NULL;

    /* 第一个空闲块为对齐后的堆地址 */
    pxFirstFreeBlock = (void *)pucAlignedHeap;
    /* 其大小为结束块的地址减去堆地址 */
    pxFirstFreeBlock->xSize = uxAddress - (size_t)pxFirstFreeBlock;
    /* 其下一个空闲块为结束快 */
    pxFirstFreeBlock->pxNext = pxEnd;

    /* 还没有进行内存分配，历史最小空闲字节数为整个可用堆空间 */
    xMinimumEverFreeBytesRemaining = pxFirstFreeBlock->xSize;
    /* 空闲字节数也为整个可用堆空间 */
    xFreeBytesRemaining = pxFirstFreeBlock->xSize;

    /* 置位最高位来表示内存已经分配（第31位） */
    xBlockAllocatedBit = ((size_t)1) << ((sizeof(size_t)* heapBITS_PER_BYTE) - 1);
}

/******************************************************************************
* 函 数 名：prvInsertBlockIntoFreeList
* 功    能：插入块节点到空闲链表中
* 传入参数：无
* 传出参数：无
* 使用举例：prvHeapInit();
*******************************************************************************/
static void prvInsertBlockIntoFreeList(BlockLink_t *pxBlockToInsert)
{
    BlockLink_t *pxIterator;    /* 迭代节点，用于遍历 */
    uint8_t *puc;               /* 临时变量存放指针 */

    /* 由于空闲块是按地址大小排列的，所以需要从头遍历找到插入点 */
    for (pxIterator = &xStart; pxIterator->pxNext < pxBlockToInsert; pxIterator = pxIterator->pxNext)
    {
        /* 这里什么都不做，只是为了遍历到插入点，遍历到了会退出循环 */
    }

    /* 查看待插入的空闲块是否在空闲链表待插入节点的相邻区域 */
    puc = (uint8_t *)pxIterator;
    if ((puc + pxIterator->xSize) == (uint8_t *)pxBlockToInsert)
    {
        /* 如果相邻就进行合并，只需要将插入点的块大小合并即可 */
        pxIterator->xSize += pxBlockToInsert->xSize;
        /* 合并后将合并后的块地址赋给待插入节点指针 */
        pxBlockToInsert = pxIterator;
    }

    /* 查看空闲块链表上当前结点后是否可以和下一个节点合并 */
    puc = (uint8_t *)pxBlockToInsert;
    if ((puc + pxBlockToInsert->xSize) == (uint8_t *)pxIterator->pxNext)
    {
        /* [a]如果相邻就进行合并，合并时需要判断是不是尾结点 */
        if (pxIterator->pxNext != pxEnd)
        {
            /* [b]如果不是尾结点，进行合并，只需要进行容量合并并删除下一节点即可 */
            pxBlockToInsert->xSize += pxIterator->pxNext->xSize;
            pxBlockToInsert->pxNext = pxIterator->pxNext->pxNext;
        }
        else
        {
            /* [b]如果是尾结点，即当前结点的下一个就是尾结点 */
            pxBlockToInsert->pxNext = pxEnd;
        }
    }
    else
    {
        /* [a]如果不相邻就将当前结点插入链表 */
        pxBlockToInsert->pxNext = pxIterator->pxNext;
    }

    /* 判断是否进行过前向合并 */
    if (pxIterator != pxBlockToInsert)
    {
        /* 如果没有，进行插入操作 */
        pxIterator->pxNext = pxBlockToInsert;
    }
}

/******************************************************************************
* 函 数 名：pvHeapMalloc
* 功    能：内存申请
* 传入参数：xWantedSize - 申请大小
* 传出参数：void * 类型的地址指针
* 使用举例：p = pvHeapMalloc(100);
*******************************************************************************/
void* pvHeapMalloc( size_t xWantedSize )
{
	BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
	void *pvReturn = NULL;

	heapINTO_CRITICAL();
	{
		/* If this is the first call to malloc then the heap will require
		initialisation to setup the list of free blocks. */
		if( pxEnd == NULL )
		{
			prvHeapInit();
		}

		if( ( xWantedSize & xBlockAllocatedBit ) == 0 )
		{
			if( xWantedSize > 0 )
			{
				xWantedSize += heapSTRUCT_SIZE;

				if( ( xWantedSize & heapALIGNMENT_MASK ) != 0x00 )
				{
					xWantedSize += ( heapALIGNMENT_BYTE - ( xWantedSize & heapALIGNMENT_MASK ) );
				}
			}

			if( ( xWantedSize > 0 ) && ( xWantedSize <= xFreeBytesRemaining ) )
			{
				/* Traverse the list from the start	(lowest address) block until
				one	of adequate size is found. */
				pxPreviousBlock = &xStart;
				pxBlock = xStart.pxNext;
				while( ( pxBlock->xSize < xWantedSize ) && ( pxBlock->pxNext != NULL ) )
				{
					pxPreviousBlock = pxBlock;
					pxBlock = pxBlock->pxNext;
				}

				/* If the end marker was reached then a block of adequate size
				was	not found. */
				if( pxBlock != pxEnd )
				{
					/* Return the memory space pointed to - jumping over the
					BlockLink_t structure at its start. */
					pvReturn = ( void * ) ( ( ( uint8_t * ) pxPreviousBlock->pxNext ) + heapSTRUCT_SIZE );

					/* This block is being returned for use so must be taken out
					of the list of free blocks. */
					pxPreviousBlock->pxNext = pxBlock->pxNext;

					/* If the block is larger than required it can be split into
					two. */
					if( ( pxBlock->xSize - xWantedSize ) > heapMINIMUM_BLOCK_SIZE )
					{
						/* This block is to be split into two.  Create a new
						block following the number of bytes requested. The void
						cast is used to prevent byte alignment warnings from the
						compiler. */
						pxNewBlockLink = ( void * ) ( ( ( uint8_t * ) pxBlock ) + xWantedSize );
						
						/* Calculate the sizes of two blocks split from the
						single block. */
						pxNewBlockLink->xSize = pxBlock->xSize - xWantedSize;
						pxBlock->xSize = xWantedSize;

						/* Insert the new block into the list of free blocks. */
						prvInsertBlockIntoFreeList( pxNewBlockLink );
					}

					xFreeBytesRemaining -= pxBlock->xSize;

					if( xFreeBytesRemaining < xMinimumEverFreeBytesRemaining )
					{
						xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
					}

					/* The block is being returned - it is allocated and owned
					by the application and has no "next" block. */
					pxBlock->xSize |= xBlockAllocatedBit;
					pxBlock->pxNext = NULL;
				}
			}
		}
	}
	heapEXIT_CRITICAL();

	return pvReturn;
}

/******************************************************************************
* 函 数 名：vHeapFree
* 功    能：内存释放
* 传入参数：pv - 待释放指针
* 传出参数：无
* 使用举例：vHeapFree(p);
*******************************************************************************/
void vHeapFree( void *pv )
{
    uint8_t *puc = (uint8_t *)pv;   /* 临时保存数据指针 */
    BlockLink_t *pxLink;            /* 空闲节点临时变量指针 */

    /* 如果待释放地址有效 */
	if( pv != NULL )
	{
        /* 得到待释放的实际节点首地址 */
		puc -= heapSTRUCT_SIZE;

		/* 获取待释放节点地址 */
		pxLink = ( void * ) puc;

        /* 检查内存是否分配 */
		if( ( pxLink->xSize & xBlockAllocatedBit ) != 0 )
		{
            /* 如果已分配*/
			if( pxLink->pxNext == NULL )
			{
				/* 取消内存分配位 */
				pxLink->xSize &= ~xBlockAllocatedBit;
                
                /* 进入临界区 */
				heapINTO_CRITICAL();
				{
					/* 更新剩余空间大小 */
					xFreeBytesRemaining += pxLink->xSize;

                    /* 将此块区域插入空闲链表 */
					prvInsertBlockIntoFreeList( ( ( BlockLink_t * ) pxLink ) );
				}
                /* 退出临界区 */
				heapEXIT_CRITICAL();
			}
		}
	}
}

/******************************************************************************
* 函 数 名：xHeapGetFreeSize
* 功    能：获取空闲内存
* 传入参数：无
* 传出参数：空闲内存大小
* 使用举例：vHeapFree(p);
*******************************************************************************/
size_t xHeapGetFreeSize(void)
{
	return xFreeBytesRemaining;
}

/******************************************************************************
* 函 数 名：xHeapGetMinimumEverFreeSize
* 功    能：获取历史最小空闲
* 传入参数：无
* 传出参数：历史最小空闲大小
* 使用举例：xHeapGetMinimumEverFreeSize(p);
*******************************************************************************/
size_t xHeapGetMinimumEverFreeSize(void)
{
	return xMinimumEverFreeBytesRemaining;
}


#endif  /* heapALLOCATION_TYPE == 4 */
