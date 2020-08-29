/*******************************************************************************
** 文件名称：heap.h
** 文件作用：动态内存管理头文件
** 编写作者：TomFree（付瑞彪）
** 编写时间：2018-04-20
** 更新时间：2018-04-20
** 文件备注：
**         本内存管理是FreeRTOS的脱离版
** 更新记录：
**         2018-04-20 -> 添加注释
*******************************************************************************/
#ifndef __HEAP_H__
#define __HEAP_H__

#include "stdlib.h"

/* 动态内存分配方案选择(1-5) */
#define	heapALLOCATION_TYPE		    ( 4 )

/* 堆的大小(字节为单位,在1024字节以上时建议采用例如 8*1024 的形式表示 8KB) */
#define	heapTOTAL_HEAP_SIZE			(size_t)( 8 * 1024 )

/* 字节对齐长度(1/2/4/8字节) */
#define heapALIGNMENT_BYTE			( 8 )

/* 是否用户自定义堆地址，如果使能，需要用户自己定义：ucHeap[heapTOTAL_HEAP_SIZE] */
#define heapUSER_ALLOCATED_HEAP		( 0 )

/* 字节对齐掩码，用于计算 */
#define heapALIGNMENT_MASK	        ( heapALIGNMENT_BYTE - 1 )

/* 进入和退出临界区，也可以创建互斥信号量 */
#define heapINTO_CRITICAL()
#define	heapEXIT_CRITICAL()

/* API函数 */
void*  pvHeapMalloc( size_t xWantedSize );
void   vHeapFree( void *pv );
size_t xHeapGetFreeSize( void );
size_t xHeapGetMinimumEverFreeSize(void);

#endif /* __HEAP_H__ */
