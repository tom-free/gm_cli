/*******************************************************************************
** 文件名称：heap1.C
** 文件作用：动态内存管理策略1源文件
** 编写作者：TomFree（付瑞彪）
** 编写时间：2018-04-20
** 更新时间：2018-04-20
** 文件备注：
**         本内存管理是FreeRTOS的动态内存管理策略1脱离版，只能进行内存的获取，
**         不可释放，
** 更新记录：
**         2018-04-20 -> 添加注释
*******************************************************************************/

#include "heap.h"

#if (heapALLOCATION_TYPE == 1)

#include "../GM_Type/gm_type.h"

/* 由于进行对齐时会出现最多heapALIGNMENT_BYTE的浪费,所以进行实际动态内存字节数校正 */
#define heapREAL_HEAP_SIZE	( heapTOTAL_HEAP_SIZE - heapALIGNMENT_BYTE )

/* 为堆分配内存 */
#if( heapUSER_ALLOCATED_HEAP == 1 )
	/* 如果用户为了将动态数据放在特殊的区域或地址,用户可以自己进行外部定义 */
	extern uint8_t ucHeap[ heapTOTAL_HEAP_SIZE ];
#else
	/* 否则系统自己定义动态数据区 */
	static uint8_t ucHeap[ heapTOTAL_HEAP_SIZE ];
#endif /* heapUSER_ALLOCATED_HEAP == 1 */

/* 此变量存储下一个空闲字节地址偏移 */
static size_t xNextFreeByte = ( size_t ) 0;

/******************************************************
* 函 数 名：pvHeapMalloc
* 功    能：动态申请内存
* 传入参数：xWantedSize -待申请内存大小
* 传出参数：void * 型的指针,申请到的地址
* 使用举例：p = (int*)pvMalloc(100);
*******************************************************/
void *pvHeapMalloc( size_t xWantedSize )
{
	void *pvReturn = NULL;
	static uint8_t *pucAlignedHeap = NULL;

/* 确保申请的空间为对齐的,如果为一字节对齐就不需要进行转换,其他对齐方式就需要进行补齐操作 */
#if ( heapALIGNMENT_BYTE != 1 )
	if( xWantedSize & heapALIGNMENT_MASK )
	{
		/* 计算出实际所需申请的字节数 */
		/* 实际所需字节数 = 待申请字节数 + 剩余字节数 */
		/* 剩余字节数可以通过取以对齐字节数为进制的个位余数(即:待申请字节数 取余 对齐字节数) */
		/* 由于字节对齐都是2^n对齐,所以取余可以使用与1操作替代而加快运行速度(即取出对应位的数据) */
		/* 例如按4字节对齐,需要申请10个字节,那实际申请字节数为:10 + (4 - (10 & 0x03)) = 12*/
		xWantedSize += ( heapALIGNMENT_BYTE - ( xWantedSize & heapALIGNMENT_MASK ) );
	}
#endif	/* portBYTE_ALIGNMENT != 1 */
	
	/* 进入临界区 */
	heapINTO_CRITICAL();
	
	/* 如果是第一次申请,即还没有指明初始地址 */
	if( pucAlignedHeap == NULL )
	{
		/* 确保对齐后的地址是与实际地址匹配的 */
		/* 这里涉及内存的寻址,寻址位数即为对齐位数 */
		/* 即CPU一次读取的字节数,所以不能出现内存的错位 */
		/* 例如ucHeap数组的首地址为0x0005,采用4字节对齐,那么前四个字节就跨了两个内存寻址区 */
		/* 0x0000-0x0003为一个寻址区,0x0004-0x0007为一个寻址区 */
		/* 所以初次申请时需要进行实际对齐区域的定位,所以采用了以下代码计算起始区 */
		/* 原理就是先跳过第一个区,直接从ucHeap[对齐字节数]进行计算,将这个地址 除于 对齐字节数(即对对齐字节数取整) */
		/* 由于字节对齐都是2^n对齐,所以取整(取整后需要还原原位数)可以使用与0操作替代而加快运行速度 */
		/* 同样上面的例子,实际起始地址为:(0x0005 + 4) & (~0x03) = 0x0008 */
		/* 更形象的计算为:(0x0005 + 4) / 4 * 4 = 0x0008 */
		/* 不是第一次申请时因为前面已经做了对齐处理,所以不需再做 */
		pucAlignedHeap = ( uint8_t * ) ( ( ( typePOINTER_SIZE ) &ucHeap[ heapALIGNMENT_BYTE ] ) & ( ~( ( typePOINTER_SIZE ) heapALIGNMENT_MASK ) ) );
	}
	
	/* 检查是否有充足的空间剩余和是否出现计数溢出 */
	if( ( ( xNextFreeByte + xWantedSize ) < heapREAL_HEAP_SIZE ) &&
		( ( xNextFreeByte + xWantedSize ) > xNextFreeByte )	)
	{
		/* 将下一个空闲地址返回 */
		pvReturn = pucAlignedHeap + xNextFreeByte;
		/* 更新标志到下一个空闲地址 */
		xNextFreeByte += xWantedSize;
	}
	
	/* 退出临界区 */
	heapEXIT_CRITICAL();
	
	/* 返回申请到的数据地址 */
	return pvReturn;
}

/******************************************************
* 函 数 名：vHeapFree
* 功    能：动态释放内存,此处不支持释放,此函数无效
* 传入参数：pv -待释放的内存地址
* 传出参数：无
* 使用举例：vPortFree(p);
*******************************************************/
void vHeapFree( void *pv )
{
	/* 使用此种内存管理无法释放内存,此种方式适用于简单的设备并且安全性较高的场合 */
	( void ) pv;	/* 防止编译器警告 */
}

/******************************************************
* 函 数 名：vHeapInit
* 功    能：初始化堆
* 传入参数：无
* 传出参数：无
* 使用举例：vHeapInit();
*******************************************************/
static void vHeapInit( void )
{
	/* 清除静态内存,复位计数标志 */
	xNextFreeByte = ( size_t ) 0;
}

/******************************************************
* 函 数 名：xGetFreeHeapSize
* 功    能：得到空闲的堆内存数
* 传入参数：无
* 传出参数：空闲字节数
* 使用举例：num = xHeapGetFreeSize();
*******************************************************/
size_t xHeapGetFreeSize( void )
{
	/* 计算剩余的空闲字节数 */
	return ( heapREAL_HEAP_SIZE - xNextFreeByte );
}

#endif  /* heapALLOCATION_TYPE == 1 */
