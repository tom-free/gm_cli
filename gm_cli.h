/*******************************************************************************
** 文件名称：gm_cli_mgr.h
** 文件作用：通用命令行接口
** 编写作者：Tom Free 付瑞彪
** 编写时间：2020-08-06
** 文件备注：
**
** 更新记录：
**          2020-08-06 -> 创建文件                             <Tom Free 付瑞彪>
**          2021-03-18 -> 修改宏来适配不同编译器               <Tom Free 付瑞彪>
**          
**              Copyright (c) 2018-2021 付瑞彪 All Rights Reserved
**
**       1 Tab == 4 Spaces     UTF-8     ANSI C Language(C99)
*******************************************************************************/
#ifndef __GM_CLI_H__
#define __GM_CLI_H__

#include "gm_cli_cfg.h"

/* 输出字符回调函数 */
typedef void gm_cli_out_char_cb_t(const char);
/* 命令执行回调函数 */
typedef int gm_cli_cmd_cb_t(int, char*[]);

/* 命令结构定义 */
typedef struct _gm_cli_cmd_t
{
    char* name;                   /* 命令名 */
    char* usage;                  /* 使用简洁说明 */
    gm_cli_cmd_cb_t *cb;          /* 命令执行函数 */
    struct _gm_cli_cmd_t *link;   /* 链接到的命令，用于命令别名 */
} gm_cli_cmd_t;

/* 编译器支持列举 */
#define GM_CLI_CC_NULL              0   /* 不支持的编译器 */
#define GM_CLI_CC_MDK_ARM           1   /* MDK for ARM (Keil) */
#define GM_CLI_CC_MDK_C51           2   /* MDK for C51 (Keil) */
#define GM_CLI_CC_IAR_ARM           3   /* IAR for ARM */
#define GM_CLI_CC_IAR_AVR           4   /* IAR for AVR */
#define GM_CLI_CC_IAR_STM8          5   /* IAR for STM8 */
#define GM_CLI_CC_GCC_LINUX         6   /* GCC for Linux */
#define GM_CLI_CC_MINGW             7   /* MinGW (GCC for Windows) */
#define GM_CLI_CC_VS                8   /* Visual Studio */

/* 编译器自动识别，不能保证100%识别正确，需要不断优化 */
#if defined (__IAR_SYSTEMS_ICC__)
#if defined (__ICCARM__)
#define GM_CLI_CC                   GM_CLI_CC_IAR_ARM
#elif defined (__ICCAVR__)
#define GM_CLI_CC                   GM_CLI_CC_IAR_AVR
#elif defined (__ICCSTM8__)
#define GM_CLI_CC                   GM_CLI_CC_IAR_STM8
#endif  /* __ICCARM__ */
#elif defined (__CC_ARM) || defined (__CLANG_ARM)
#define GM_CLI_CC                   GM_CLI_CC_MDK_ARM
#elif defined (__C51__)
#define GM_CLI_CC                   GM_CLI_CC_MDK_C51
#elif defined __GNUC__
#if defined (__WIN32__)
#define GM_CLI_CC                   GM_CLI_CC_MINGW
#elif defined (__linux__)
#define GM_CLI_CC                   GM_CLI_CC_GCC_LINUX
#endif
#elif defined(_MSC_VER)
#define GM_CLI_CC                   GM_CLI_CC_VS
#else
#define GM_CLI_CC                   GM_CLI_CC_NULL
#error "Do not support this compiler"
#endif  /* __IAR_SYSTEMS_ICC__ */

/* 字符串连接 */
#define GM_CLI_STR_CONNECT2(a, b)       a ## b
#define GM_CLI_STR_CONNECT3(a, b, c)    a ## b ## c

#if (GM_CLI_CC == GM_CLI_CC_MDK_ARM)
/* 导出命令 */
#define GM_CLI_CMD_EXPORT(cmd_name, cmd_usage, cmd_cb)                         \
        __attribute__((used)) __attribute__((section("gm_cli_cmd_section")))   \
            static const gm_cli_cmd_t                                          \
            GM_CLI_STR_CONNECT2(gm_cli_cmd_, cmd_name) =                       \
            {                                                                  \
                .name    = #cmd_name,                                          \
                .usage   = cmd_usage,                                          \
                .cb      = cmd_cb,                                             \
                .link    = NULL,                                               \
            };
/* 命令命别名 */
#define GM_CLI_CMD_ALIAS_NUM(cmd_name, cmd_alias_str, num)                     \
        __attribute__((used)) __attribute__((section("gm_cli_cmd_section")))   \
            static const gm_cli_cmd_t                                          \
            GM_CLI_STR_CONNECT3(gm_cli_cmd_, cmd_name##_, num) =               \
            {                                                                  \
                .name    = cmd_alias_str,                                      \
                .usage   = NULL,                                               \
                .cb      = NULL,                                               \
                .link    = (gm_cli_cmd_t*)&gm_cli_cmd_##cmd_name,              \
            };
/* 命令别名 */
#define GM_CLI_CMD_ALIAS(cmd_name, cmd_alias_str)                              \
        GM_CLI_CMD_ALIAS_NUM(cmd_name, cmd_alias_str, __LINE__)
#endif

#if (GM_CLI_CC == GM_CLI_CC_MDK_C51)
#endif

#if (GM_CLI_CC == GM_CLI_CC_IAR_ARM)
/* 定义相关段 */
#pragma section=".gm_cli_cmd_section"
/* 导出命令 */
#define GM_CLI_CMD_EXPORT(cmd_name, cmd_usage, cmd_cb)                         \
        _Pragma("location = \".gm_cli_cmd_section\"")                          \
            static __root const gm_cli_cmd_t gm_cli_cmd_##cmd_name =           \
            {                                                                  \
                .name    = #cmd_name,                                          \
                .usage   = cmd_usage,                                          \
                .cb      = cmd_cb,                                             \
            };
#endif

#if (GM_CLI_CC == GM_CLI_CC_IAR_AVR)
#endif

#if (GM_CLI_CC == GM_CLI_CC_IAR_STM8)
#endif

#if (GM_CLI_CC == GM_CLI_CC_GCC_LINUX)
/* 导出命令 */
#define GM_CLI_CMD_EXPORT(cmd_name, cmd_usage, cmd_cb)                         \
        __attribute__((used)) __attribute__((section("gm_cli_cmd_section")))   \
            static const gm_cli_cmd_t                                          \
            GM_CLI_STR_CONNECT2(gm_cli_cmd_, cmd_name) =                       \
            {                                                                  \
                .name    = #cmd_name,                                          \
                .usage   = cmd_usage,                                          \
                .cb      = cmd_cb,                                             \
                .link    = NULL,                                               \
            };
/* 命令命别名 */
#define GM_CLI_CMD_ALIAS_NUM(cmd_name, cmd_alias_str, num)                     \
        __attribute__((used)) __attribute__((section("gm_cli_cmd_section")))   \
            static const gm_cli_cmd_t                                          \
            GM_CLI_STR_CONNECT3(gm_cli_cmd_, cmd_name##_, num) =               \
            {                                                                  \
                .name    = cmd_alias_str,                                      \
                .usage   = NULL,                                               \
                .cb      = NULL,                                               \
                .link    = (gm_cli_cmd_t*)&gm_cli_cmd_##cmd_name,              \
            };
/* 命令别名 */
#define GM_CLI_CMD_ALIAS(cmd_name, cmd_alias_str)                              \
        GM_CLI_CMD_ALIAS_NUM(cmd_name, cmd_alias_str, __LINE__)
#endif

#if (GM_CLI_CC == GM_CLI_CC_MINGW)
/* 导出命令 */
#define GM_CLI_CMD_EXPORT(cmd_name, cmd_usage, cmd_cb)                         \
        __attribute__((used)) __attribute__((section(".gm_cli_cmd_section$b")))\
            static const gm_cli_cmd_t                                          \
            GM_CLI_STR_CONNECT2(gm_cli_ex_cmd_, cmd_name) =                    \
            {                                                                  \
                .name    = #cmd_name,                                          \
                .usage   = cmd_usage,                                          \
                .cb      = cmd_cb,                                             \
            };
/* 命令命别名，需要指定一个编号来区分名称 */
#define GM_CLI_CMD_ALIAS_NUM(cmd_name, cmd_alias_str, num)                     \
        __attribute__((used)) __attribute__((section(".gm_cli_cmd_section$b")))\
            static const gm_cli_cmd_t                                          \
            GM_CLI_STR_CONNECT3(gm_cli_ex_cmd_, cmd_name##_, num) =            \
            {                                                                  \
                .name    = cmd_alias_str,                                      \
                .usage   = NULL,                                               \
                .cb      = NULL,                                               \
                .link    = (gm_cli_cmd_t*)&gm_cli_ex_cmd_##cmd_name,           \
            };
/* 命令别名 */
#define GM_CLI_CMD_ALIAS(cmd_name, cmd_alias_str)                              \
        GM_CLI_CMD_ALIAS_NUM(cmd_name, cmd_alias_str, __LINE__)
#endif

#if (GM_CLI_CC == GM_CLI_CC_VS)
/* 定义三个段，a和c用来作为搜索的起止位置，b用来存放实际命令 */
#pragma section(".gm_cli_cmd_section$a", read)
#pragma section(".gm_cli_cmd_section$b", read)
#pragma section(".gm_cli_cmd_section$c", read)
/* 导出命令 */
#define GM_CLI_CMD_EXPORT(cmd_name, cmd_usage, cmd_cb)                         \
        __declspec(allocate(".gm_cli_cmd_section$b"))                          \
            static const gm_cli_cmd_t                                          \
            GM_CLI_STR_CONNECT2(gm_cli_ex_cmd_, cmd_name) =                    \
            {                                                                  \
                .name    = #cmd_name,                                          \
                .usage   = cmd_usage,                                          \
                .cb      = cmd_cb,                                             \
                .link    = NULL,                                               \
            };
/* 命令命别名，需要指定一个编号来区分名称 */
#define GM_CLI_CMD_ALIAS_NUM(cmd_name, cmd_alias_str, num)                     \
        __declspec(allocate(".gm_cli_cmd_section$b"))                          \
            static const gm_cli_cmd_t                                          \
            GM_CLI_STR_CONNECT3(gm_cli_ex_cmd_, cmd_name##_, num) =            \
            {                                                                  \
                .name    = cmd_alias_str,                                      \
                .usage   = NULL,                                               \
                .cb      = NULL,                                               \
                .link    = (gm_cli_cmd_t*)&gm_cli_ex_cmd_##cmd_name,           \
            };
/* 命令别名 */
#define GM_CLI_CMD_ALIAS(cmd_name, cmd_alias_str)                              \
        GM_CLI_CMD_ALIAS_NUM(cmd_name, cmd_alias_str, __LINE__)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
** 函数名称：gm_cli_mgr_init
** 函数作用：初始化CLI管理器
** 输入参数：无
** 输出参数：无
** 使用范例：gm_cli_mgr_init();
** 函数备注：
*******************************************************************************/
void gm_cli_mgr_init(void);

/*******************************************************************************
** 函数名称：gm_cli_set_out_char_cb
** 函数作用：设置输出字符回调函数
** 输入参数：out_char_cb - 输出一个字符回调函数
** 输出参数：无
** 使用范例：gm_cli_set_out_char_cb(fun_out_char);
** 函数备注：
*******************************************************************************/
void gm_cli_set_out_char_cb(gm_cli_out_char_cb_t *out_char_cb);

/*******************************************************************************
** 函数名称：gm_cli_set_cmd_prompt
** 函数作用：设置命令提示符
** 输入参数：p_notice - 提示符
** 输出参数：无
** 使用范例：gm_cli_set_cmd_prompt("[CMD] > ");
** 函数备注：
*******************************************************************************/
void gm_cli_set_cmd_prompt(const char* const p_notice);

/*******************************************************************************
** 函数名称：gm_cli_start
** 函数作用：启动CLI
** 输入参数：无
** 输出参数：无
** 使用范例：gm_cli_start();
** 函数备注：
*******************************************************************************/
void gm_cli_start(void);

/*******************************************************************************
** 函数名称：gm_cli_put_char
** 函数作用：打印字符
** 输入参数：ch - 字符
** 输出参数：无
** 使用范例：gm_cli_put_char('A');
** 函数备注：
*******************************************************************************/
void gm_cli_put_char(const char ch);

/*******************************************************************************
** 函数名称：gm_cli_put_str
** 函数作用：打印字符串
** 输入参数：str
** 输出参数：无
** 使用范例：gm_cli_put_str("hello\r\n");
** 函数备注：
*******************************************************************************/
void gm_cli_put_str(const char* const str);

/*******************************************************************************
** 函数名称：gm_cli_printf
** 函数作用：打印函数
** 输入参数：fmt - 格式化字符串
**           ... - 可变参数
** 输出参数：无
** 使用范例：gm_cli_printf("%d\r\n", 123);
** 函数备注：
*******************************************************************************/
void gm_cli_printf(const char* const fmt, ...);

/*******************************************************************************
** 函数名称：gm_cli_parse_char
** 函数作用：解析一个字符
** 输入参数：ch - 字符
** 输出参数：无
** 使用样例：gm_cli_parse_char('\r');
** 函数备注：
*******************************************************************************/
void gm_cli_parse_char(const char ch);

#ifdef __cplusplus
}
#endif

#endif  /* __GM_CLI_H__ */
