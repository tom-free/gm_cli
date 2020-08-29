/*******************************************************************************
** 文件名称：heap3.C
** 文件作用：动态内存管理策略3源文件
** 编写作者：TomFree（付瑞彪）
** 编写时间：2018-04-20
** 更新时间：2018-04-20
** 文件备注：
**         本内存管理是FreeRTOS的脱离版
** 更新记录：
**         2018-04-20 -> 添加注释
*******************************************************************************/

#include "heap.h"

/* 如果选择的是第三种内存管理策略 */
#if (heapALLOCATION_TYPE == 3)

/******************************************************
* 函 数 名：pvHeapMalloc
* 功    能：动态内存申请
* 传入参数：xWantedSize -需要的内存大小
* 传出参数：void * 型的地址
* 使用举例：p = pvHeapMalloc(100);
*******************************************************/
void *pvHeapMalloc( size_t xWantedSize )
{
	void *pvReturn;

	heapINTO_CRITICAL();

	pvReturn = malloc( xWantedSize );

	heapEXIT_CRITICAL();

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
	if(pv)
	{
		heapINTO_CRITICAL();

		free( pv );

		heapEXIT_CRITICAL();
	}
}

#endif  /* heapALLOCATION_TYPE == 3 */
