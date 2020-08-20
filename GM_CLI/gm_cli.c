/*******************************************************************************
** 文件名称：gm_cli.c
** 文件作用：通用命令行接口
** 编写作者：Tom Free 付瑞彪
** 编写时间：2020-08-09
** 文件备注：
**
** 更新记录：
**          2020-08-09 -> 创建文件                             <Tom Free 付瑞彪>
**
**              Copyright (c) 2018-2020 付瑞彪 All Rights Reserved
**
**       1 Tab == 4 Spaces     UTF-8     ANSI C Language(C99)
*******************************************************************************/

#include "gm_cli.h"
#include "string.h"
#include "stdio.h"

#if defined (_MSC_VER)
/* Microsoft VC/C++ 编译器没有找到段起始和终止的操作宏，需要特殊处理 */
__declspec(allocate(".gm_cli_cmd_section$a"))
static const GM_CLI_CMD __gm_cli_cmd_begin =
{
    .name = "__start",
    .usage = "start of cli",
    .cb = NULL,
};
__declspec(allocate(".gm_cli_cmd_section$c"))
static const GM_CLI_CMD __gm_cli_cmd_end =
{
    .name = "__end",
    .usage = "end of cli",
    .cb = NULL,
};
#endif  /* _MSC_VER */

/* 默认命令提示符 */
#define GM_CLI_DEFAULT_CMD_NITICE   "[General CLI] > "

/* 输入状态定义 */
typedef enum
{
    GM_CLI_INPUT_WAIT_NORMAL,       /* 等待正常字符 */
    GM_CLI_INPUT_WAIT_SPEC_KEY,     /* 等待特殊字符 */
    GM_CLI_INPUT_WAIT_FUNC_KEY,     /* 等待功能字符 */
    GM_CLI_INPUT_WAIT_FUNC_KEY1,    /* 等待功能字符1 */
} GM_CLI_INPUT_STATUS;

/* CLI管理器 */
typedef struct
{
    char                line[GM_CLI_LINE_CHAR_MAX]; /* 一行字符串存储 */
    int                 input_count;                /* 输入的字符数量 */
    int                 input_cusor;                /* 输入的光标位置 */
    GM_CLI_INPUT_STATUS input_status;               /* 当前输入的状态 */
    GM_CLI_OUT_CHAR_CB  pf_outchar;                 /* 输出字符回调函数 */
    const GM_CLI_CMD*   p_cmd_start;                /* 命令存储区起始指针 */
    const GM_CLI_CMD*   p_cmd_end;                  /* 命令存储区结束指针 */
    const char*         p_cmd_notice;               /* 命令提示符 */
} GM_CLI;

/* CLI控制 */
static GM_CLI gm_cli =
{
    .input_count = 0,
    .input_cusor = 0,
    .input_status = GM_CLI_INPUT_WAIT_NORMAL,
    .pf_outchar = NULL,
    .p_cmd_start = NULL,
    .p_cmd_end = NULL,
    .p_cmd_notice = GM_CLI_DEFAULT_CMD_NITICE,
};

/*******************************************************************************
** 函数名称：GM_CLI_Init
** 函数作用：初始化CLI
** 输入参数：无
** 输出参数：无
** 使用范例：GM_CLI_Init();
** 函数备注：
*******************************************************************************/
void GM_CLI_Init(void)
{
#if defined (_MSC_VER)
    unsigned int* ptr_begin, *ptr_end;

    /* 找寻起始位置 */
    ptr_begin = (unsigned int*)&__gm_cli_cmd_begin;
    ptr_begin += (sizeof(GM_CLI_CMD) / sizeof(unsigned int));
    while ((*ptr_begin) == 0) ptr_begin++;

    /* 找寻终止位置 */
    ptr_end = (unsigned int *)&__gm_cli_cmd_end;
    ptr_end--;
    while ((*ptr_end) == 0) ptr_end--;

    /* 判断是否合法 */
    if (ptr_begin < ptr_end)
    {
        gm_cli.p_cmd_start = (GM_CLI_CMD*)(ptr_begin);
        gm_cli.p_cmd_end   = (GM_CLI_CMD*)(ptr_end);
    }
    else
    {
        gm_cli.p_cmd_start = NULL;
        gm_cli.p_cmd_end = NULL;
    }
#elif defined (__IAR_SYSTEMS_ICC__)
    gm_cli.p_cmd_start = (GM_CLI_CMD*)__section_begin(".gm_cli_cmd_section");
    gm_cli.p_cmd_end   = (GM_CLI_CMD*)__section_end(".gm_cli_cmd_section");
#endif

    gm_cli.input_count = 0;
    gm_cli.input_cusor = 0;
    gm_cli.input_status = GM_CLI_INPUT_WAIT_NORMAL;
    gm_cli.pf_outchar = NULL;
    gm_cli.p_cmd_notice = GM_CLI_DEFAULT_CMD_NITICE;
}

/*******************************************************************************
** 函数名称：GM_CLI_RegOutCharCallBack
** 函数作用：注册输出字符回调
** 输入参数：pf_outchar -
** 输出参数：
** 使用范例：GM_CLI_RegOutCharCallBack();
** 函数备注：
*******************************************************************************/
void GM_CLI_RegOutCharCallBack(void(*pf_outchar)(const char))
{
    if (pf_outchar != NULL)
    {
        gm_cli.pf_outchar = pf_outchar;
    }
}

/*******************************************************************************
** 函数名称：GM_CLI_SetCommandNotice
** 函数作用：设置命令提示符
** 输入参数：p_notice - 提示符
** 输出参数：无
** 使用范例：GM_CLI_SetCommandNotice();
** 函数备注：
*******************************************************************************/
void GM_CLI_SetCommandNotice(const char* const p_notice)
{
    if (p_notice != NULL)
    {
        gm_cli.p_cmd_notice = p_notice;
    }
}

/*******************************************************************************
** 函数名称：GM_CLI_FindCommand
** 函数作用：命令查找
** 输入参数：cmd_name - 命令名
** 输出参数：命令结构体指针
** 使用样例：GM_CLI_FindCommand("help");
** 函数备注：
*******************************************************************************/
static const GM_CLI_CMD* GM_CLI_FindCommand(const char* const cmd_name)
{
    const GM_CLI_CMD* p_ret = NULL;
    const GM_CLI_CMD* p_temp;
    
    for (p_temp = gm_cli.p_cmd_start; p_temp < gm_cli.p_cmd_end; p_temp++)
    {
        if (strcmp(p_temp->name, cmd_name) == 0)
        {
            p_ret = p_temp;
            break;
        }
    }

    return p_ret;
}

/*******************************************************************************
** 函数名称：GM_CLI_PutChar
** 函数作用：打印字符
** 输入参数：ch - 字符
** 输出参数：无
** 使用范例：GM_CLI_PutChar();
** 函数备注：
*******************************************************************************/
static void GM_CLI_PutChar(const char ch)
{
    if (gm_cli.pf_outchar != NULL)
    {
        gm_cli.pf_outchar(ch);
    }
}

/*******************************************************************************
** 函数名称：GM_CLI_PutString
** 函数作用：打印字符串
** 输入参数：str
** 输出参数：无
** 使用范例：GM_CLI_PutString();
** 函数备注：
*******************************************************************************/
static void GM_CLI_PutString(const char* const str)
{
    const char* ptemp = str;
    if (str == NULL)
    {
        return;
    }
    if (gm_cli.pf_outchar != NULL)
    {
        while (*ptemp)
        {
            gm_cli.pf_outchar(*ptemp);
            ptemp++;
        }
    }
}

/*******************************************************************************
** 函数名称：GM_CLI_Parse_UpKey
** 函数作用：上键
** 输入参数：无
** 输出参数：无
** 使用范例：GM_CLI_Parse_UpKey();
** 函数备注：
*******************************************************************************/
static void GM_CLI_Parse_UpKey(void)
{
    // TODO 
    /* 上翻历史 */
}

/*******************************************************************************
** 函数名称：GM_CLI_Parse_DownKey
** 函数作用：下键
** 输入参数：无
** 输出参数：无
** 使用范例：GM_CLI_Parse_DownKey();
** 函数备注：
*******************************************************************************/
static void GM_CLI_Parse_DownKey(void)
{
    // TODO 
    /* 下翻历史 */
}

/*******************************************************************************
** 函数名称：GM_CLI_Parse_BackSpace
** 函数作用：左键
** 输入参数：无
** 输出参数：无
** 使用范例：GM_CLI_Parse_LeftKey();
** 函数备注：
*******************************************************************************/
static void GM_CLI_Parse_LeftKey(void)
{
    if (gm_cli.input_cusor > 0)
    {
        GM_CLI_PutChar('\b');
        gm_cli.input_cusor--;
    }
}

/*******************************************************************************
** 函数名称：GM_CLI_Parse_RightKey
** 函数作用：右键
** 输入参数：无
** 输出参数：无
** 使用范例：GM_CLI_Parse_RightKey();
** 函数备注：
*******************************************************************************/
static void GM_CLI_Parse_RightKey(void)
{
    if ((gm_cli.input_count > 0) &&
        (gm_cli.input_cusor < gm_cli.input_count))
    {
        GM_CLI_PutChar(gm_cli.line[gm_cli.input_cusor]);
        gm_cli.input_cusor++;
    }
}

/*******************************************************************************
** 函数名称：GM_CLI_Parse_FuncKey
** 函数作用：功能键过滤
** 输入参数：ch - 字符
** 输出参数：无
** 使用范例：GM_CLI_Parse_FuncKey();
** 函数备注：
*******************************************************************************/
static int GM_CLI_Parse_FuncKey(const char ch)
{
    /* XSHELL终端，超级终端等功能码 */
    if (ch == (const char)0x1B)
    {
        gm_cli.input_status = GM_CLI_INPUT_WAIT_SPEC_KEY;
        return 0;
    }
    else if (gm_cli.input_status == GM_CLI_INPUT_WAIT_SPEC_KEY)
    {
        if (ch == (const char)0x5b)
        {
            gm_cli.input_status = GM_CLI_INPUT_WAIT_FUNC_KEY;
            return 0;
        }

        gm_cli.input_status = GM_CLI_INPUT_WAIT_NORMAL;
    }
    else if (gm_cli.input_status == GM_CLI_INPUT_WAIT_FUNC_KEY)
    {
        gm_cli.input_status = GM_CLI_INPUT_WAIT_NORMAL;

        if (ch == (const char)0x41)      /* 上 */
        {
            return 0;
        }
        else if (ch == (const char)0x42) /* 下 */
        {
            return 0;
        }
        else if (ch == (const char)0x44) /* 左 */
        {
            GM_CLI_Parse_LeftKey();
            return 0;
        }
        else if (ch == (const char)0x43) /* 右 */
        {
            GM_CLI_Parse_RightKey();
            return 0;
        }
    }

    /* windows命令行功能码 */
    if (ch == (const char)0xE0)
    {
        gm_cli.input_status = GM_CLI_INPUT_WAIT_FUNC_KEY1;
        return 0;
    }
    else if (gm_cli.input_status == GM_CLI_INPUT_WAIT_FUNC_KEY1)
    {
        gm_cli.input_status = GM_CLI_INPUT_WAIT_NORMAL;
        if (ch == (const char)0x48)         /* 上 */
        {
            GM_CLI_Parse_UpKey();
            return 0;
        }
        else if (ch == (const char)0x50)    /* 下 */
        {
            GM_CLI_Parse_DownKey();
            return 0;
        }
        else if (ch == (const char)0x4B)    /* 左 */
        {
            GM_CLI_Parse_LeftKey();
            return 0;
        }
        else if (ch == (const char)0x4D)    /* 右 */
        {
            GM_CLI_Parse_RightKey();
            return 0;
        }
    }

    return -1;
}

/*******************************************************************************
** 函数名称：GM_CLI_Parse_TabKey
** 函数作用：Tab键
** 输入参数：无
** 输出参数：无
** 使用范例：GM_CLI_Parse_TabKey();
** 函数备注：
*******************************************************************************/
static void GM_CLI_Parse_TabKey(void)
{

}

/*******************************************************************************
** 函数名称：GM_CLI_Parse_BackSpace
** 函数作用：Tab键
** 输入参数：无
** 输出参数：无
** 使用范例：GM_CLI_Parse_BackSpace();
** 函数备注：
*******************************************************************************/
static void GM_CLI_Parse_BackSpace(void)
{

}

/*******************************************************************************
** 函数名称：GM_CLI_Parse_Enter
** 函数作用：回车或换行
** 输入参数：无
** 输出参数：无
** 使用范例：GM_CLI_Parse_Enter();
** 函数备注：
*******************************************************************************/
static void GM_CLI_Parse_Enter(void)
{

}

/*******************************************************************************
** 函数名称：GM_CLI_Parse_CommonChar
** 函数作用：通用可显示字符
** 输入参数：无
** 输出参数：无
** 使用范例：GM_CLI_Parse_CommonChar();
** 函数备注：
*******************************************************************************/
static void GM_CLI_Parse_CommonChar(const char ch)
{
    gm_cli.line[gm_cli.input_cusor++] = ch;
    gm_cli.input_count++;
    GM_CLI_PutChar(ch);
}

/*******************************************************************************
** 函数名称：GM_CLI_ParseOneChar
** 函数作用：解析一个字符
** 输入参数：ch - 字符
** 输出参数：无
** 使用样例：GM_CLI_ParseOneChar('\r');
** 函数备注：
*******************************************************************************/
void GM_CLI_ParseOneChar(const char ch)
{
    /* 过滤无效字符 */
    if ((ch == (const char)0x00) ||
        (ch == (const char)0xFF))
    {
        return;
    }

    /* 功能码 */
    if (GM_CLI_Parse_FuncKey(ch) == 0)
    {
        return;
    }
    
    /* 字符解析 */
    if (ch == '\t')
    {
        /* Tab */
        GM_CLI_Parse_TabKey();
    }
    else if ((ch == (const char)0x7F) || (ch == (const char)0x08))
    {
        /* 退格 */
        GM_CLI_Parse_BackSpace();
    }
    if ((ch == '\r') || (ch == '\n'))
    {
        /* 回车或换行 */
        GM_CLI_Parse_Enter();
    }
    else if ((ch > ' ') && (ch < '~'))
    {
        GM_CLI_Parse_CommonChar(ch);
    }
}

/*******************************************************************************
** 函数名称：GM_CLI_CMD_help
** 函数作用：系统命令帮助指令
** 输入参数：argc - 参数个数
**           argv - 参数列表
** 输出参数：执行结果，0 - 成功，其他 - 错误
** 使用范例：导出到系统，搜索到指令自动执行
** 函数备注：
*******************************************************************************/
static int GM_CLI_CMD_help(int argc, char* argv[])
{
    const GM_CLI_CMD* p_ret = NULL;
    const GM_CLI_CMD* p_temp;

    for (p_temp = gm_cli.p_cmd_start; p_temp < gm_cli.p_cmd_end; p_temp++)
    {
        GM_CLI_PutString(p_temp->name);
        GM_CLI_PutString("\r\n");
    }
    return 0;
}
GM_CLI_CMD_EXPORT(help, "help", GM_CLI_CMD_help);
