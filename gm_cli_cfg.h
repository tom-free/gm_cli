/*******************************************************************************
** 文件名称：gm_cli_cfg.h
** 文件作用：通用命令行接口配置文件
** 编写作者：Tom Free 付瑞彪
** 编写时间：2021-03-18
** 文件备注：
**
** 更新记录：
**          2021-03-18 -> 创建文件                             <Tom Free 付瑞彪>
**
**              Copyright (c) 2018-2021 付瑞彪 All Rights Reserved
**
**       1 Tab == 4 Spaces     UTF-8     ANSI C Language(C99)
*******************************************************************************/
#ifndef __GM_CLI_CFG_H__
#define __GM_CLI_CFG_H__

/* 是否通过编译器段自动注册命令
 * 1、如果编译器是系统已配置可以使用的编译器就可以配置此处为1，然后可以使用
 *    EXPORT和ALIAS宏来导出命令，这个也是最好的方式，可以降低文件耦合性，但需要
 *    了解编译器段分配原理和使用方式，目前系统仅支持几个主流编译器；
 * 2、如果编译器是系统为支持的就必须配置此处为0，然后在应用层提供注册的命令表，
 *    此种方式可以支持任意的编译器，但会增加程序耦合性 */
#define GM_CLI_CMD_REG_BY_CC_SECTION    0u

/* 一行的最大输入字符数，包括一个\0，实际需要减一 */
#define GM_CLI_LINE_CHAR_MAX            64u

/* 最长的输入命令参数总数，包括命令本身 */
#define GM_CLI_CMD_ARGS_NUM_MAX         5u

/* 打印函数buf大小，需要大于能够打印的最大长度，建议使用打印函数时长度不要过长 */
#define GM_CLI_PRINTF_BUF_MAX           128u

/* 最大的备份行数，用于历史记录 */
#define GM_CLI_HISTORY_LINE_MAX         10u

/* 默认命令提示符 */
#define GM_CLI_DEFAULT_CMD_PROMPT       "[CLI] > "

#endif  /* __GM_CLI_CFG_H__ */
