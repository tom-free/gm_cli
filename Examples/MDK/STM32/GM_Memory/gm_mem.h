/*******************************************************************************
** 文件名称：gm_mem.h
** 文件作用：内存管理
** 编写作者：Tom Free 付瑞彪
** 编写时间：2018-08-25
** 文件备注：
**
**
** 更新记录：
**          2018-08-25 -> 创建文件                                   <Tom Free>
**
**          1 Tab == 4 Spaces     UTF-8     ANSI C Language
*******************************************************************************/
#ifndef __GM_MEM_H__
#define __GM_MEM_H__

#include "string.h"
#include "stdlib.h"
#include "heap.h"

#define GM_Mem_Alloc                    pvHeapMalloc
#define GM_Mem_Free                     vHeapFree
#define GM_Mem_GetFreeSize              xHeapGetFreeSize
#define GM_Mem_GetMinimumEverFreeSize   xHeapGetMinimumEverFreeSize

#define GM_Mem_Copy(dst, src, len)      memcpy(dst, src, len)
#define GM_Mem_Set(dst, src, data)      memset(dst, src, data)

#endif  /* __GM_MEM_H__ */
