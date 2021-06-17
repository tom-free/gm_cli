/*******************************************************************************
** 文件名称：gm_cli.h
** 文件作用：通用命令行接口
** 编写作者：Tom Free 付瑞彪
** 编写时间：2020-08-06
** 文件备注：
**
** 更新记录：
**          2020-08-06 -> 创建文件                             <Tom Free 付瑞彪>
**
**              Copyright (c) 2018-2020 付瑞彪 All Rights Reserved
**
**       1 Tab == 4 Spaces     UTF-8     ANSI C Language(C99)
*******************************************************************************/
#ifndef __GM_CLI_H__
#define __GM_CLI_H__

//静态命令行控制宏定义
#define GM_CLI_USING_STATIC_CMD 1

#if defined(_MSC_VER)
/* 定义三个段 */
#pragma section(".gm_cli_cmd_section$a", read)
#pragma section(".gm_cli_cmd_section$b", read)
#pragma section(".gm_cli_cmd_section$c", read)
#elif defined (__IAR_SYSTEMS_ICC__)
/* 定义相关段 */
#pragma section=".gm_cli_cmd_section"
#endif

/* 一行的最大输入字符数，包括一个\0，实际需要减一 */
#define GM_CLI_LINE_CHAR_MAX            64u
/* 最长的输入命令参数总数，包括命令本身 */
#define GM_CLI_CMD_ARGS_NUM_MAX         5u
/* 打印函数buf大小，需要大于能够打印的最大长度，建议使用打印函数时长度不要过长 */
#define GM_CLI_PRINTF_BUF_MAX           128u
/* 最大的备份行数，用于历史记录 */
#define GM_CLI_HISTORY_LINE_MAX         10u
/* 命令字符串最大长度 */
#define GM_CLI_CMD_NAME_MAX_LENGTH		16u

/* 命令结构 */
typedef struct
{
    char* name;             /* 命令名 */
    char* usage;            /* 使用简洁说明 */
    int(*cb)(int, char*[]); /* 命令执行函数 */
} GM_CLI_CMD;

#if !GM_CLI_USING_STATIC_CMD

#if defined(__CC_ARM) || defined(__CLANG_ARM)          /* ARM C Compiler */
/* 导出命令 */
#define GM_CLI_CMD_EXPORT(cmd_name, cmd_usage, cmd_cb)                         \
        __attribute__((used)) __attribute__((section("gm_cli_cmd_section")))	 \
            static  const GM_CLI_CMD gm_cli_cmd_##cmd_name =             \
            {                                                                  \
                .name    = #cmd_name,                                          \
                .usage   = cmd_usage,                                          \
                .cb      = cmd_cb,                                             \
            };
#elif defined (_MSC_VER)                                      /* 微软Windows */
/* 导出命令 */
#define GM_CLI_CMD_EXPORT(cmd_name, cmd_usage, cmd_cb)                         \
        __declspec(allocate(".gm_cli_cmd_section$b"))                          \
            static const GM_CLI_CMD gm_cli_cmd_##cmd_name =                    \
            {                                                                  \
                .name    = #cmd_name,                                          \
                .usage   = cmd_usage,                                          \
                .cb      = cmd_cb,                                             \
            };
#elif defined (__IAR_SYSTEMS_ICC__) || defined(__ICCARM__)  /* IAR */
/* 导出命令 */
#define GM_CLI_CMD_EXPORT(cmd_name, cmd_usage, cmd_cb)                         \
        _Pragma("location = \".gm_cli_cmd_section\"")                          \
            static __root const GM_CLI_CMD gm_cli_cmd_##cmd_name =             \
            {                                                                  \
                .name    = #cmd_name,                                          \
                .usage   = cmd_usage,                                          \
                .cb      = cmd_cb,                                             \
            };
#elif defined (__GNUC__)
/* 导出命令 */
#define GM_CLI_CMD_EXPORT(cmd_name, cmd_usage, cmd_cb)                         \
        __attribute__((used)) __attribute__((section(".gm_cli_cmd_section$b")))\
            static const GM_CLI_CMD gm_cli_cmd_##cmd_name =                    \
            {                                                                  \
                .name    = #cmd_name,                                          \
                .usage   = cmd_usage,                                          \
                .cb      = cmd_cb,                                             \
            };
#else
#error "not support this compiler"
#endif
#endif


/* 输出字符回调 */
typedef void(*GM_CLI_OUT_CHAR_CB)(const char);

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
** 函数名称：GM_CLI_Init
** 函数作用：初始化CLI
** 输入参数：无
** 输出参数：无
** 使用范例：GM_CLI_Init();
** 函数备注：
*******************************************************************************/
void GM_CLI_Init(void);

/*******************************************************************************
** 函数名称：GM_CLI_RegOutCharCallBack
** 函数作用：注册输出字符回调
** 输入参数：pf_outchar -
** 输出参数：
** 使用范例：GM_CLI_RegOutCharCallBack();
** 函数备注：
*******************************************************************************/
void GM_CLI_RegOutCharCallBack(void(*pf_outchar)(const char));

/*******************************************************************************
** 函数名称：GM_CLI_SetCommandPrompt
** 函数作用：设置命令提示符
** 输入参数：p_notice - 提示符
** 输出参数：无
** 使用范例：GM_CLI_SetCommandPrompt();
** 函数备注：
*******************************************************************************/
void GM_CLI_SetCommandPrompt(const char* const p_notice);

/*******************************************************************************
** 函数名称：GM_CLI_Start
** 函数作用：启动CLI
** 输入参数：无
** 输出参数：无
** 使用范例：GM_CLI_Start();
** 函数备注：
*******************************************************************************/
void GM_CLI_Start(void);

/*******************************************************************************
** 函数名称：GM_CLI_PutChar
** 函数作用：打印字符
** 输入参数：ch - 字符
** 输出参数：无
** 使用范例：GM_CLI_PutChar();
** 函数备注：
*******************************************************************************/
void GM_CLI_PutChar(const char ch);

/*******************************************************************************
** 函数名称：GM_CLI_PutString
** 函数作用：打印字符串
** 输入参数：str
** 输出参数：无
** 使用范例：GM_CLI_PutString();
** 函数备注：
*******************************************************************************/
void GM_CLI_PutString(const char* const str);

/*******************************************************************************
** 函数名称：GM_CLI_Printf
** 函数作用：打印函数
** 输入参数：fmt - 格式化字符串
**           ... - 可变参数
** 输出参数：无
** 使用范例：GM_CLI_Printf("%d\r\n", 123);
** 函数备注：
*******************************************************************************/
void GM_CLI_Printf(const char* const fmt, ...);

/*******************************************************************************
** 函数名称：GM_CLI_ParseOneChar
** 函数作用：解析一个字符
** 输入参数：ch - 字符
** 输出参数：无
** 使用样例：GM_CLI_ParseOneChar('\r');
** 函数备注：
*******************************************************************************/
void GM_CLI_ParseOneChar(const char ch);


#if GM_CLI_USING_STATIC_CMD
/*******************************************************************************
** 函数名称：GM_CLI_RegStaticCMDs
** 函数作用：静态方式注册命令表
** 输入参数：cmds - 命令表指针
** 输出参数：无
** 使用样例：GM_CLI_RegStaticCMDs(&user_cmds[0]);
** 函数备注：
*******************************************************************************/
void GM_CLI_RegStaticCMDs(GM_CLI_CMD *cmds);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* __GM_CLI_H__ */
