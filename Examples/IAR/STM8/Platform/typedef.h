/*******************************************************************************
** 文件名称：typedef.h
** 文件作用：通用类型定义头文件，防止不同平台的数据类型差异
** 编写作者：Tom Free 付瑞彪
** 编写时间：2018-11-01
** 文件备注：自定义通用数据类型，方便移植，使用了标准的数据类型，如：uint8_t
**			 为了和标准库以及常用命名区分，自定义数据类型都带有“GM_”前缀，但为
**			 了兼容设计，有时也保留了没有此前缀的宏或数据类型，一般建议优先选
**			 择带有“GM_”前缀的宏或数据类型
**
** 更新记录：
**          2018-11-01 -> 创建文件                          <Tom Free 付瑞彪>
**
**
**       Copyright (c) 深圳市三派智能科技有限公司 All Rights Reserved
**
**       1 Tab == 4 Spaces     UTF-8     ANSI C Language(C99)
*******************************************************************************/
#ifndef __TYPEDEF_H__
#define __TYPEDEF_H__

/* STM8L头文件 */
#include "stm8l15x_conf.h"

/******************************************************************************/
/********************************** 配置区域 **********************************/
/* 配置项：是否使用位域来定义数据类型，0 - 不使用，1 - 使用 */
#define GM_BIT_FIELD	1

/* 配置项：单片机内存存放模式，0 - 小端模式，1 - 大端模式 */
#define GM_MEM_TYPE     1

/* 配置项：是否支持64位数据类型，部分编译器不支持，0 - 不支持，1 - 支持 */
#define GM_64BITS_TYPE  0
/************************************* END ************************************/

/* 系统常用空指针定义，用于兼容其他人的模块，不建议使用此宏 */
#ifndef NULL
#define NULL	((void*)0)
#endif

/* 自定义空指针，建议使用此宏 */
#define GM_NULL	((void*)0)

/* 计算成员偏移地址 */
#define GM_OFFSETOF(TYPE, MEMBER)   ((size_t)&((TYPE *)0)->MEMBER)

/* BOOL类型定义 */
typedef enum
{
    GM_FALSE   = 0,
    GM_TRUE    = !GM_FALSE,

    GM_DISABLE = 0,
    GM_ENABLE  = !GM_DISABLE,

    GM_OFF     = 0,
    GM_ON      = !GM_OFF,

	GM_FAIL    = 0,
    GM_SUCCESS = !GM_FAIL,

    GM_ERR     = 0,
    GM_OK      = !GM_ERR
} GM_BOOL;

/* 无符号基本数据类型重定义 */
typedef uint8_t  GM_U8;     /*  8位 */
typedef uint16_t GM_U16;    /* 16位 */
typedef uint32_t GM_U32;    /* 32位 */

/* 部分编译系统不支持此类型 */
#if GM_64BITS_TYPE
typedef uint64_t GM_U64;    /* 64位 */
#endif  /* GM_64BITS_TYPE */

/* 有符号基本数据类型重定义 */
typedef int8_t   GM_S8;     /*  8位 */
typedef int16_t  GM_S16;    /* 16位 */
typedef int32_t  GM_S32;    /* 32位 */

/* 部分编译系统不支持此类型 */
#if GM_64BITS_TYPE
typedef int64_t  GM_S64;    /* 64位 */
#endif  /* GM_64BITS_TYPE */

/* 尺寸大小类型定义，16位，最大65535 */
typedef uint16_t   GM_SIZE;

/* 通用回调函数 */
typedef GM_BOOL GM_CALLBACK(void* args, GM_U32 arg);

/* 通用初始化函数 */
typedef void GM_INIT_FUNC(void);

/* 时间结构体 */
typedef struct
{
    GM_U8  year;    /* 年，  0 - 255*/
    GM_U8  month;   /* 月，  1 - 12 */
    GM_U8  day;     /* 日，  1 - 31 */
    GM_U8  week;    /* 星期，1 - 7 */
    GM_U8  hour;    /* 时，  0 - 23 */
    GM_U8  minute;  /* 分，  0 - 60 */
    GM_U8  second;  /* 秒，  0 - 60 */
} GM_TIME;

/* 以下需要配置使能位域支持，仅支持C99以上编译器
 * 对于多字节类型同时要注意大小端模式 */
#if GM_BIT_FIELD

/* 定义字节类型 */
typedef union
{
    uint8_t byte;           /* 字节 */
    struct
    {
        uint8_t bit0 : 1;   /* 位0 */
        uint8_t bit1 : 1;
        uint8_t bit2 : 1;
        uint8_t bit3 : 1;
        uint8_t bit4 : 1;
        uint8_t bit5 : 1;
        uint8_t bit6 : 1;
        uint8_t bit7 : 1;   /* 位7 */
    };
} GM_BYTE;

/* 定义字类型 */
typedef union
{
    uint16_t word;          /* 字 */
    uint8_t  byte[2];       /* 两字节 */
    struct
    {

#if GM_MEM_TYPE == 0
        uint8_t bit0 : 1;   /* 位0 */
        uint8_t bit1 : 1;
        uint8_t bit2 : 1;
        uint8_t bit3 : 1;
        uint8_t bit4 : 1;
        uint8_t bit5 : 1;
        uint8_t bit6 : 1;
        uint8_t bit7 : 1;
        uint8_t bit8 : 1;
        uint8_t bit9 : 1;
        uint8_t bit10 : 1;
        uint8_t bit11 : 1;
        uint8_t bit12 : 1;
        uint8_t bit13 : 1;
        uint8_t bit14 : 1;
        uint8_t bit15 : 1;  /* 位15 */
#else
        uint8_t bit8 : 1;   /* 位8 */
        uint8_t bit9 : 1;
        uint8_t bit10 : 1;
        uint8_t bit11 : 1;
        uint8_t bit12 : 1;
        uint8_t bit13 : 1;
        uint8_t bit14 : 1;
        uint8_t bit15 : 1;  /* 位15 */
        uint8_t bit0 : 1;   /* 位0 */
        uint8_t bit1 : 1;
        uint8_t bit2 : 1;
        uint8_t bit3 : 1;
        uint8_t bit4 : 1;
        uint8_t bit5 : 1;
        uint8_t bit6 : 1;
        uint8_t bit7 : 1;   /* 位7 */
#endif
    };
} GM_WORD;

/* 定义双字类型 */
typedef union
{
    uint32_t dword;         /* 双字 */
    uint16_t word[2];       /* 两个字 */
    uint8_t  byte[4];       /* 四个字节 */
    struct
    {
#if GM_MEM_TYPE == 0
        uint8_t bit0 : 1;   /* 位0 */
        uint8_t bit1 : 1;
        uint8_t bit2 : 1;
        uint8_t bit3 : 1;
        uint8_t bit4 : 1;
        uint8_t bit5 : 1;
        uint8_t bit6 : 1;
        uint8_t bit7 : 1;
        uint8_t bit8 : 1;
        uint8_t bit9 : 1;
        uint8_t bit10 : 1;
        uint8_t bit11 : 1;
        uint8_t bit12 : 1;
        uint8_t bit13 : 1;
        uint8_t bit14 : 1;
        uint8_t bit15 : 1;
        uint8_t bit16 : 1;
        uint8_t bit17 : 1;
        uint8_t bit18 : 1;
        uint8_t bit19 : 1;
        uint8_t bit20 : 1;
        uint8_t bit21 : 1;
        uint8_t bit22 : 1;
        uint8_t bit23 : 1;
        uint8_t bit24 : 1;
        uint8_t bit25 : 1;
        uint8_t bit26 : 1;
        uint8_t bit27 : 1;
        uint8_t bit28 : 1;
        uint8_t bit29 : 1;
        uint8_t bit30 : 1;
        uint8_t bit31 : 1;  /* 位31 */
#else
        uint8_t bit24 : 1;  /* 位24 */
        uint8_t bit25 : 1;
        uint8_t bit26 : 1;
        uint8_t bit27 : 1;
        uint8_t bit28 : 1;
        uint8_t bit29 : 1;
        uint8_t bit30 : 1;
        uint8_t bit31 : 1;  /* 位31 */
        uint8_t bit16 : 1;  /* 位16 */
        uint8_t bit17 : 1;
        uint8_t bit18 : 1;
        uint8_t bit19 : 1;
        uint8_t bit20 : 1;
        uint8_t bit21 : 1;
        uint8_t bit22 : 1;
        uint8_t bit23 : 1;  /* 位23 */
        uint8_t bit8 : 1;   /* 位8 */
        uint8_t bit9 : 1;
        uint8_t bit10 : 1;
        uint8_t bit11 : 1;
        uint8_t bit12 : 1;
        uint8_t bit13 : 1;
        uint8_t bit14 : 1;
        uint8_t bit15 : 1;  /* 位15 */
        uint8_t bit0 : 1;   /* 位0 */
        uint8_t bit1 : 1;
        uint8_t bit2 : 1;
        uint8_t bit3 : 1;
        uint8_t bit4 : 1;
        uint8_t bit5 : 1;
        uint8_t bit6 : 1;
        uint8_t bit7 : 1;   /* 位7 */
#endif
    };
}GM_DWORD;

#endif	/* GM_BIT_FIELD */

#endif	/* TYPEDEF_H */