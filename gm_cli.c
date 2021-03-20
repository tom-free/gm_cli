/*******************************************************************************
** 文件名称：gm_cli_mgr.c
** 文件作用：通用命令行接口
** 编写作者：Tom Free 付瑞彪
** 编写时间：2020-08-09
** 文件备注：
**
** 更新记录：
**          2020-08-09 -> 创建文件                             <Tom Free 付瑞彪>
**          2021-03-18 -> 修改宏来适配不同编译器               <Tom Free 付瑞彪>
**
**              Copyright (c) 2018-2021 付瑞彪 All Rights Reserved
**
**       1 Tab == 4 Spaces     UTF-8     ANSI C Language(C99)
*******************************************************************************/

#include "gm_cli.h"
#include "string.h"
#include "stdio.h"
#include "stdarg.h"

#if (GM_CLI_CC == GM_CLI_CC_VS)
/* Microsoft VC/C++ 编译器没有找到段起始和终止的操作宏，需要特殊处理 */
__declspec(allocate(".gm_cli_cmd_section$a"))
static const gm_cli_cmd_t __gm_cli_cmd_begin =
{
    .name = "__start",
    .usage = "start of cli",
    .cb = NULL,
};
__declspec(allocate(".gm_cli_cmd_section$c"))
static const gm_cli_cmd_t __gm_cli_cmd_end =
{
    .name = "__end",
    .usage = "end of cli",
    .cb = NULL,
};
#elif (GM_CLI_CC == GM_CLI_CC_MINGW)
__attribute__((used)) __attribute__((section(".gm_cli_cmd_section$a")))
static const gm_cli_cmd_t __gm_cli_cmd_begin =
{
    .name = "__start",
    .usage = "start of cli",
    .cb = NULL,
};
__attribute__((used)) __attribute__((section(".gm_cli_cmd_section$c")))
static const gm_cli_cmd_t __gm_cli_cmd_end =
{
    .name = "__end",
    .usage = "end of cli",
    .cb = NULL,
};
#endif  /* GM_CLI_CC == GM_CLI_CC_VS */

/* 输入状态定义 */
typedef enum _gm_cli_input_status_t
{
    GM_CLI_INPUT_WAIT_NORMAL,       /* 等待正常字符 */
    GM_CLI_INPUT_WAIT_SPEC_KEY,     /* 等待特殊字符 */
    GM_CLI_INPUT_WAIT_FUNC_KEY,     /* 等待功能字符 */
#if (GM_CLI_CC == GM_CLI_CC_VS) || (GM_CLI_CC == GM_CLI_CC_MINGW)
    GM_CLI_INPUT_WAIT_FUNC_KEY1,    /* 等待功能字符1 */
#endif
} gm_cli_input_status_t;

/* CLI管理器 */
typedef struct _gm_cli_mgr_t
{
    char                  line[GM_CLI_LINE_CHAR_MAX];    /* 一行字符串存储 */
    unsigned int          input_count;                   /* 输入的字符数量 */
    unsigned int          input_cusor;                   /* 输入的光标位置 */
    gm_cli_input_status_t input_status;                  /* 当前输入的状态 */
    gm_cli_out_char_cb_t *pf_outchar;                    /* 输出字符回调函数 */
    const int*            p_cmd_start;                   /* 命令存储区起始指针 */
    const int*            p_cmd_end;                     /* 命令存储区结束指针 */
    const char*           p_cmd_notice;                  /* 命令提示符 */
    /* 打印函数使用的字符串缓存 */
    char                  printf_str[GM_CLI_PRINTF_BUF_MAX];
    /* 备份字符串，用于翻历史记录时保存当前 */
    char                  backup_str[GM_CLI_LINE_CHAR_MAX];
    char                  history_str[GM_CLI_HISTORY_LINE_MAX][GM_CLI_LINE_CHAR_MAX];
    unsigned int          history_total;                 /* 历史总记录条数 */
    unsigned int          history_index;                 /* 历史存储索引 */
    unsigned int          history_inquire_index;         /* 历史查询索引 */
    unsigned int          history_inquire_count;         /* 历史查询数量计数器 */
} gm_cli_mgr_t;

/* CLI控制 */
static gm_cli_mgr_t gm_cli_mgr =
{
    .input_count = 0,
    .input_cusor = 0,
    .input_status = GM_CLI_INPUT_WAIT_NORMAL,
    .pf_outchar = NULL,
    .p_cmd_start = NULL,
    .p_cmd_end = NULL,
    .p_cmd_notice = GM_CLI_DEFAULT_CMD_PROMPT,
    .history_total = 0,
    .history_index = 0,
    .history_inquire_index = 0,
    .history_inquire_count =0,
};

/* 读取下一个命令 */
static const gm_cli_cmd_t* gm_cli_get_next_cmd(const int* const addr)
{
#if ((GM_CLI_CC == GM_CLI_CC_MDK_ARM)   || \
     (GM_CLI_CC == GM_CLI_CC_IAR_ARM)   || \
     (GM_CLI_CC == GM_CLI_CC_MINGW)     || \
     (GM_CLI_CC == GM_CLI_CC_GCC_LINUX))
    const int* ptr = (const int*)((int)addr + sizeof(gm_cli_cmd_t));
    if (ptr < gm_cli_mgr.p_cmd_end)
    {
        return (const gm_cli_cmd_t*)ptr;
    }
#elif (GM_CLI_CC == GM_CLI_CC_VS)
    const int* ptr = addr;
    ptr += sizeof(gm_cli_cmd_t) / sizeof(const int);
    while (ptr < gm_cli_mgr.p_cmd_end)
    {
        if ((*ptr) != 0)
        {
            return (const gm_cli_cmd_t*)ptr;
        }
        ptr++;
    }
#endif

    return NULL;
}

/* 搜索命令 */
static const gm_cli_cmd_t* gm_cli_search_cmd(const char* const cmd_name)
{
    const gm_cli_cmd_t* p_ret = NULL;
    const gm_cli_cmd_t* p_temp;

    p_temp = (gm_cli_cmd_t*)gm_cli_mgr.p_cmd_start;

    while (p_temp != NULL)
    {
        if (strcmp(p_temp->name, cmd_name) == 0)
        {
            p_ret = p_temp;
            break;
        }
        p_temp = gm_cli_get_next_cmd((const int*)p_temp);
    }

    return p_ret;
}

/* 初始化cli管理器 */
void gm_cli_mgr_init(void)
{
#if (GM_CLI_CC == GM_CLI_CC_MDK_ARM) || (GM_CLI_CC == GM_CLI_CC_GCC_LINUX)
    extern const int gm_cli_cmd_section$$Base;
    extern const int gm_cli_cmd_section$$Limit;
    gm_cli_mgr.p_cmd_start = (const int*)&gm_cli_cmd_section$$Base;
    gm_cli_mgr.p_cmd_end   = (const int*)&gm_cli_cmd_section$$Limit;
#elif (GM_CLI_CC == GM_CLI_CC_IAR_ARM)
    gm_cli_mgr.p_cmd_start = (const int*)__section_begin(".gm_cli_cmd_section");
    gm_cli_mgr.p_cmd_end = (const int*)__section_end(".gm_cli_cmd_section");
#elif (GM_CLI_CC == GM_CLI_CC_VS) || (GM_CLI_CC == GM_CLI_CC_MINGW)
    unsigned int* ptr_begin, *ptr_end;

    /* 找寻起始位置 */
    ptr_begin = (unsigned int*)&__gm_cli_cmd_begin;
    ptr_begin += (sizeof(gm_cli_cmd_t) / sizeof(unsigned int));
    while ((*ptr_begin) == 0) ptr_begin++;

    /* 找寻终止位置 */
    ptr_end = (unsigned int *)&__gm_cli_cmd_end;
    ptr_end--;
    while ((*ptr_end) == 0) ptr_end--;

    /* 判断是否合法 */
    if (ptr_begin < ptr_end)
    {
        gm_cli_mgr.p_cmd_start = (const int*)(ptr_begin);
        gm_cli_mgr.p_cmd_end   = (const int*)(ptr_end);
    }
    else
    {
        gm_cli_mgr.p_cmd_start = NULL;
        gm_cli_mgr.p_cmd_end   = NULL;
    }
#endif

    gm_cli_mgr.input_count = 0;
    gm_cli_mgr.input_cusor = 0;
    gm_cli_mgr.input_status = GM_CLI_INPUT_WAIT_NORMAL;
    gm_cli_mgr.pf_outchar = NULL;
    gm_cli_mgr.p_cmd_notice = GM_CLI_DEFAULT_CMD_PROMPT;

    memset(gm_cli_mgr.line, 0, sizeof(gm_cli_mgr.line));
    memset(gm_cli_mgr.printf_str, 0, sizeof(gm_cli_mgr.printf_str));
    memset(gm_cli_mgr.backup_str, 0, sizeof(gm_cli_mgr.backup_str));
    memset(gm_cli_mgr.history_str, 0, sizeof(gm_cli_mgr.history_str));
}

/* 设置输出字符回调函数 */
void gm_cli_set_out_char_cb(gm_cli_out_char_cb_t *out_char_cb)
{
    if (out_char_cb != NULL)
    {
        gm_cli_mgr.pf_outchar = out_char_cb;
    }
}

/* 设置命令提示符 */
void gm_cli_set_cmd_prompt(const char* const p_notice)
{
    if (p_notice != NULL)
    {
        gm_cli_mgr.p_cmd_notice = p_notice;
    }
    else
    {
        gm_cli_mgr.p_cmd_notice = GM_CLI_DEFAULT_CMD_PROMPT;
    }
}

/* 启动命令行 */
void gm_cli_start(void)
{
    gm_cli_put_str("\r\n");
    gm_cli_put_str(gm_cli_mgr.p_cmd_notice);
}

/* 打印字符 */
void gm_cli_put_char(const char ch)
{
    if (gm_cli_mgr.pf_outchar != NULL)
    {
        gm_cli_mgr.pf_outchar(ch);
    }
}

/* 打印字符串 */
void gm_cli_put_str(const char* const str)
{
    const char* ptemp = str;
    if (str == NULL)
    {
        return;
    }
    if (gm_cli_mgr.pf_outchar != NULL)
    {
        while (*ptemp)
        {
            gm_cli_mgr.pf_outchar(*ptemp);
            ptemp++;
        }
    }
}

/* 通用打印函数，替代默认printf */
void gm_cli_printf(const char* const fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    
#if (GM_CLI_CC == GM_CLI_CC_VS)
    vsprintf_s(gm_cli_mgr.printf_str, sizeof(gm_cli_mgr.printf_str), fmt, ap);
#else
    vsprintf(gm_cli_mgr.printf_str, fmt, ap);
#endif
    gm_cli_put_str(gm_cli_mgr.printf_str);

    va_end(ap);
}

/* 上键处理 */
static void gm_cli_parse_up_key(void)
{
    unsigned int len;

    if (gm_cli_mgr.history_total == 0)
    {
        /* 无记录 */
        return;
    }

    if (gm_cli_mgr.history_inquire_count == 0)
    {
        /* 从未上翻记录，备份当前输入 */
        memcpy(gm_cli_mgr.backup_str, gm_cli_mgr.line, sizeof(gm_cli_mgr.line));
        /* 搜索记录位置设置到当前记录处 */
        gm_cli_mgr.history_inquire_index = gm_cli_mgr.history_index;
    }
    
    /* 查看是否已经搜索完成 */
    if (gm_cli_mgr.history_inquire_count < gm_cli_mgr.history_total)
    {
        /* 搜索编号往前找一个记录编号 */
        if (gm_cli_mgr.history_inquire_index == 0)
        {
            gm_cli_mgr.history_inquire_index = GM_CLI_HISTORY_LINE_MAX - 1;
        }
        else
        {
            gm_cli_mgr.history_inquire_index--;
        }
        /* 搜索数量加1 */
        gm_cli_mgr.history_inquire_count++;
        /* 光标之后的行数据清除掉 */
        len = gm_cli_mgr.input_count - gm_cli_mgr.input_cusor;
        for (unsigned int i = 0; i < len; i++)
        {
            gm_cli_put_char(' ');
        }
        /* 清除所有的行数据 */
        for (unsigned int i = 0; i < gm_cli_mgr.input_count; i++)
        {
            gm_cli_put_str("\b \b");
        }

        /* 导入历史输入 */
        memcpy(gm_cli_mgr.line, gm_cli_mgr.history_str[gm_cli_mgr.history_inquire_index], sizeof(gm_cli_mgr.line));
        gm_cli_mgr.input_count = (unsigned int)strlen(gm_cli_mgr.line);
        gm_cli_mgr.input_cusor = gm_cli_mgr.input_count;
        /* 显示历史记录 */
        gm_cli_put_str(gm_cli_mgr.line);
    }
}

/* 下键处理 */
static void gm_cli_parse_down_key(void)
{
    unsigned int len;

    if ((gm_cli_mgr.history_total == 0) || 
        (gm_cli_mgr.history_inquire_count == 0))
    {
        /* 无记录或无上翻 */
        return;
    }

    /* 查询数量减一 */
    gm_cli_mgr.history_inquire_count--;
    /* 删除当前行内容 */
    len = gm_cli_mgr.input_count - gm_cli_mgr.input_cusor;
    for (unsigned int i = 0; i < len; i++)
    {
        gm_cli_put_char(' ');
    }
    for (unsigned int i = 0; i < gm_cli_mgr.input_count; i++)
    {
        gm_cli_put_str("\b \b");
    }

    if (gm_cli_mgr.history_inquire_count == 0)
    {
        /* 恢复备份的输入 */
        memcpy(gm_cli_mgr.line, gm_cli_mgr.backup_str, sizeof(gm_cli_mgr.line));
    }
    else
    {
        /* 更新索引 */
        gm_cli_mgr.history_inquire_index++;
        gm_cli_mgr.history_inquire_index %= GM_CLI_HISTORY_LINE_MAX;
        /* 取出历史 */
        memcpy(gm_cli_mgr.line, gm_cli_mgr.history_str[gm_cli_mgr.history_inquire_index], sizeof(gm_cli_mgr.line));
    }

    /* 重新更新坐标 */
    gm_cli_mgr.input_count = (unsigned int)strlen(gm_cli_mgr.line);
    gm_cli_mgr.input_cusor = gm_cli_mgr.input_count;
    /* 显示输入行 */
    gm_cli_put_str(gm_cli_mgr.line);
}

/* 左键处理 */
static void gm_cli_parse_left_key(void)
{
    if (gm_cli_mgr.input_cusor > 0)
    {
        gm_cli_put_char('\b');
        gm_cli_mgr.input_cusor--;
    }
}

/* 右键处理 */
static void gm_cli_parse_right_key(void)
{
    if ((gm_cli_mgr.input_count > 0) &&
        (gm_cli_mgr.input_cusor < gm_cli_mgr.input_count))
    {
        gm_cli_put_char(gm_cli_mgr.line[gm_cli_mgr.input_cusor]);
        gm_cli_mgr.input_cusor++;
    }
}

/* 功能键处理返回：0 - 已处理的功能字符，-1 - 未处理的字符 */
static int gm_cli_parse_func_key(const char ch)
{
    /* XSHELL终端，超级终端等功能码 */
    if (ch == (char)0x1B)
    {
        gm_cli_mgr.input_status = GM_CLI_INPUT_WAIT_SPEC_KEY;
        return 0;
    }
    else if (gm_cli_mgr.input_status == GM_CLI_INPUT_WAIT_SPEC_KEY)
    {
        if (ch == (char)0x5b)
        {
            gm_cli_mgr.input_status = GM_CLI_INPUT_WAIT_FUNC_KEY;
            return 0;
        }

        gm_cli_mgr.input_status = GM_CLI_INPUT_WAIT_NORMAL;
    }
    else if (gm_cli_mgr.input_status == GM_CLI_INPUT_WAIT_FUNC_KEY)
    {
        gm_cli_mgr.input_status = GM_CLI_INPUT_WAIT_NORMAL;

        if (ch == (char)0x41)      /* 上 */
        {
            gm_cli_parse_up_key();
            return 0;
        }
        else if (ch == (char)0x42) /* 下 */
        {
            gm_cli_parse_down_key();
            return 0;
        }
        else if (ch == (char)0x44) /* 左 */
        {
            gm_cli_parse_left_key();
            return 0;
        }
        else if (ch == (char)0x43) /* 右 */
        {
            gm_cli_parse_right_key();
            return 0;
        }
    }

#if (GM_CLI_CC == GM_CLI_CC_VS) || (GM_CLI_CC == GM_CLI_CC_MINGW)
    /* windows命令行功能码 */
    if (ch == (char)0xE0)
    {
        gm_cli_mgr.input_status = GM_CLI_INPUT_WAIT_FUNC_KEY1;
        return 0;
    }
    else if (gm_cli_mgr.input_status == GM_CLI_INPUT_WAIT_FUNC_KEY1)
    {
        gm_cli_mgr.input_status = GM_CLI_INPUT_WAIT_NORMAL;
        if (ch == (char)0x48)         /* 上 */
        {
            gm_cli_parse_up_key();
            return 0;
        }
        else if (ch == (char)0x50)    /* 下 */
        {
            gm_cli_parse_down_key();
            return 0;
        }
        else if (ch == (char)0x4B)    /* 左 */
        {
            gm_cli_parse_left_key();
            return 0;
        }
        else if (ch == (char)0x4D)    /* 右 */
        {
            gm_cli_parse_right_key();
            return 0;
        }
    }
#endif  /* _MSC_VER */

    return -1;
}

/* 判断字符串是否空，返回：0 - 空，-1 - 非空 */
static int gm_cli_str_empty_check(const char* const str)
{
    const char* p_tmp = str;
    while (*p_tmp)
    {
        if ((*p_tmp) != ' ')
        {
            return -1;
        }
        p_tmp++;
    }
    return 0;
}

/* 字符串匹配 */
static int gm_cli_str_completion(const char* const str, const char* const substr)
{
    unsigned int len1 = (unsigned int)strlen(str);
    unsigned int len2 = (unsigned int)strlen(substr);
    unsigned int i = 0;

    if (len2 > len1)
    {
        return -1;
    }
    for (i = 0; i < len2; i++)
    {
        if (str[i] != substr[i])
        {
            return -1;
        }
    }
    return 0;
}

/* 删除开头的空格 */
static const char* gm_cli_delete_start_space(const char* const str)
{
    unsigned int i = 0;
    unsigned int len = (unsigned int)strlen(str);
    while ((str[i] == ' ') && (str[i] != '\0') && (i < len))
    {
        i++;
    }
    return (const char*)(str + i);
}

/* tab键处理 */
static void gm_cli_parse_tab_key(void)
{
    const gm_cli_cmd_t *p_temp, *p_find_first_cmd = NULL;
    unsigned int find_count = 0, len;
    const char *p_line_start;

    /* 检测是否是空白行 */
    if (gm_cli_str_empty_check(gm_cli_mgr.line) == 0)
    {
        return;
    }

    /* 取消前面的空白 */
    p_line_start = gm_cli_delete_start_space(gm_cli_mgr.line);

    p_temp = (gm_cli_cmd_t*)gm_cli_mgr.p_cmd_start;

    /* 查询命令 */
    while (p_temp != NULL)
    {
        if (gm_cli_str_completion(p_temp->name, p_line_start) == 0)
        {
            if (find_count == 0)
            {
                p_find_first_cmd = p_temp;
            }
            else if (find_count == 1)
            {
                gm_cli_put_str("\r\n");
                gm_cli_put_str(p_find_first_cmd->name);
                gm_cli_put_str("\r\n");
                gm_cli_put_str(p_temp->name);
                gm_cli_put_str("\r\n");
            }
            else
            {
                gm_cli_put_str(p_temp->name);
                gm_cli_put_str("\r\n");
            }
            find_count++;
        }
        p_temp = gm_cli_get_next_cmd((const int*)p_temp);
    }

    if (find_count == 1)
    {
        /* 删除当前行内容 */
        len = gm_cli_mgr.input_count - gm_cli_mgr.input_cusor;
        for (unsigned int i = 0; i < len; i++)
        {
            gm_cli_put_char(' ');
        }
        for (unsigned int i = 0; i < gm_cli_mgr.input_count; i++)
        {
            gm_cli_put_str("\b \b");
        }
        
        /* 自动填充行 */
        memset(gm_cli_mgr.line, '\0', sizeof(gm_cli_mgr.line));
        memcpy(gm_cli_mgr.line, p_find_first_cmd->name, strlen(p_find_first_cmd->name));
        /* 重新更新坐标 */
        gm_cli_mgr.input_count = (unsigned int)strlen(gm_cli_mgr.line);
        gm_cli_mgr.input_cusor = gm_cli_mgr.input_count;
        /* 显示输入行 */
        gm_cli_put_str(gm_cli_mgr.line);
    }
    else if (find_count > 1)
    {
        /* 显示提示符 */
        gm_cli_put_str(gm_cli_mgr.p_cmd_notice);
        /* 重新更新坐标 */
        gm_cli_mgr.input_count = (unsigned int)strlen(gm_cli_mgr.line);
        gm_cli_mgr.input_cusor = gm_cli_mgr.input_count;
        /* 显示输入行 */
        gm_cli_put_str(gm_cli_mgr.line);
    }
}

/* 退格键处理 */
static void gm_cli_parse_backspace_key(void)
{
    int i, count;

    if (gm_cli_mgr.input_cusor == 0)
    {
        /* 未输入，直接跳过 */
        return;
    }

    /* 更新位置 */
    gm_cli_mgr.input_cusor--;
    gm_cli_mgr.input_count--;

    if (gm_cli_mgr.input_cusor == gm_cli_mgr.input_count)
    {
        /* 末尾置0 */
        gm_cli_mgr.line[gm_cli_mgr.input_count] = '\0';
        /* 光标在最后 */
        gm_cli_put_str("\b \b");
    }
    else
    {
        /* 计算需要搬移的字符数 */
        count = gm_cli_mgr.input_count - gm_cli_mgr.input_cusor;
        /* 搬移 */
        for (i = 0; i < count; i++)
        {
            gm_cli_mgr.line[gm_cli_mgr.input_cusor + i] = gm_cli_mgr.line[gm_cli_mgr.input_cusor + i + 1];
        }
        /* 末尾置0 */
        gm_cli_mgr.line[gm_cli_mgr.input_count] = '\0';
        /* 重新刷新显示 */
        gm_cli_put_char('\b');
        gm_cli_put_str(&gm_cli_mgr.line[gm_cli_mgr.input_cusor]);
        gm_cli_put_str(" \b");
        /* 光标回位 */
        for (i = 0; i < count; i++)
        {
            gm_cli_put_char('\b');
        }
    }
}

/* 回车换行键处理 */
static void gm_cli_parse_enter_key(void)
{
    unsigned int i;
    int argc = 0;
    char* argv[GM_CLI_CMD_ARGS_NUM_MAX];
    const gm_cli_cmd_t* p_cmd;

    /* 回车，处理命令时可能有输出 */
    gm_cli_put_str("\r\n");

    if (gm_cli_mgr.input_count > 0)
    {
        /* 备份进入历史记录 */
        memcpy(gm_cli_mgr.history_str[gm_cli_mgr.history_index++], gm_cli_mgr.line, sizeof(gm_cli_mgr.line));
        gm_cli_mgr.history_index %= GM_CLI_HISTORY_LINE_MAX;
        if (gm_cli_mgr.history_total < GM_CLI_HISTORY_LINE_MAX)
        {
            gm_cli_mgr.history_total++;
        }
        gm_cli_mgr.history_inquire_index = 0;
        gm_cli_mgr.history_inquire_count = 0;

        /* 分析字符串 */
        for (i = 0; i < gm_cli_mgr.input_count;)
        {
            /* 跳过空格并替换为0 */
            while ((gm_cli_mgr.line[i] == ' ') && (i < gm_cli_mgr.input_count))
            {
                gm_cli_mgr.line[i++] = '\0';
            }
            if (i >= gm_cli_mgr.input_count)
            {
                break;
            }

            if (argc >= GM_CLI_CMD_ARGS_NUM_MAX)
            {
                gm_cli_put_str("Too many args! Line will replace follow:\r\n  < ");
                for (int j = 0; j < argc; j++)
                {
                    gm_cli_put_str(argv[j]);
                    gm_cli_put_char(' ');
                }
                gm_cli_put_str(">\r\n");
                break;
            }

            argv[argc++] = &gm_cli_mgr.line[i];
            /* 跳过中间的字符串 */
            while ((gm_cli_mgr.line[i] != ' ') && (i < gm_cli_mgr.input_count))
            {
                i++;
            }
            if (i >= gm_cli_mgr.input_count)
            {
                break;
            }
        }

        if (argc > 0)
        {
            p_cmd = gm_cli_search_cmd(argv[0]);
            if (p_cmd != NULL)
            {
                if (p_cmd->link != NULL)
                {
                    p_cmd = p_cmd->link;
                }
                if (p_cmd->cb)
                {
                    p_cmd->cb(argc, argv);
                }
            }
            else
            {
                gm_cli_put_str("Not found command \"");
                gm_cli_put_str(argv[0]);
                gm_cli_put_str("\"\r\n");
            }
        }
    }

    /* 清空行，为下一次输入准备 */
    gm_cli_put_str(gm_cli_mgr.p_cmd_notice);
    memset(gm_cli_mgr.line, 0, sizeof(gm_cli_mgr.line));
    gm_cli_mgr.input_cusor = gm_cli_mgr.input_count = 0;
}

/* 通用可显示字符处理 */
static void gm_cli_parse_common_char(const char ch)
{
    int i, count;

    if (gm_cli_mgr.input_count > (GM_CLI_LINE_CHAR_MAX - 1))
    {
        return;
    }
    if (gm_cli_mgr.input_cusor == gm_cli_mgr.input_count)
    {
        /* 光标在最后 */
        gm_cli_mgr.line[gm_cli_mgr.input_cusor++] = ch;
        gm_cli_mgr.input_count++;
        gm_cli_put_char(ch);
    }
    else
    {
        /* 光标不在最后，计算需要搬移的字符数 */
        count = gm_cli_mgr.input_count - gm_cli_mgr.input_cusor;
        /* 搬移 */
        for (i = count; i > 0; i--)
        {
            gm_cli_mgr.line[gm_cli_mgr.input_cusor + i] = gm_cli_mgr.line[gm_cli_mgr.input_cusor + i - 1];
        }
        gm_cli_put_char(ch);
        gm_cli_mgr.input_count++;
        gm_cli_mgr.input_cusor++;
        gm_cli_put_str(&gm_cli_mgr.line[gm_cli_mgr.input_cusor]);
        /* 光标回位 */
        for (i = 0; i < count; i++)
        {
            gm_cli_put_char('\b');
        }
    }
}

/* 解析一个字符 */
void gm_cli_parse_char(const char ch)
{
    /* 过滤无效字符 */
    if ((ch == (char)0x00) ||
        (ch == (char)0xFF))
    {
        return;
    }

    /* 功能码 */
    if (gm_cli_parse_func_key(ch) == 0)
    {
        return;
    }

    /* 字符解析 */
    if (ch == '\t')
    {
        /* Tab */
        gm_cli_parse_tab_key();
    }
    else if ((ch == (char)0x7F) || (ch == (char)0x08))
    {
        /* 退格 */
        gm_cli_parse_backspace_key();
    }
    if ((ch == '\r') || (ch == '\n'))
    {
        /* 回车或换行 */
        gm_cli_parse_enter_key();
    }
    else if ((ch >= ' ') && (ch <= '~'))
    {
        gm_cli_parse_common_char(ch);
    }
}

/* 内部命令-help */
static int gm_cli_internal_cmd_help(int argc, char* argv[])
{
    const gm_cli_cmd_t* p_temp;
    const gm_cli_cmd_t* p_temp1;
    int   found_flag = 0;

    if (argc == 1)
    {
        p_temp = (gm_cli_cmd_t*)gm_cli_mgr.p_cmd_start;

        gm_cli_put_str("System all command:\r\n");
        while (p_temp != NULL)
        {
            gm_cli_put_str("    ");
            gm_cli_put_str(p_temp->name);
            if (p_temp->link != NULL)
            {
                gm_cli_put_str(" -> ");
                p_temp1 = p_temp->link;
                gm_cli_put_str(p_temp1->name);
            }
            gm_cli_put_str("\r\n");
            p_temp = gm_cli_get_next_cmd((const int*)p_temp);
        }
    }
    else if (argc == 2)
    {
        p_temp = (gm_cli_cmd_t*)gm_cli_mgr.p_cmd_start;
        found_flag = 0;
        while (p_temp != NULL)
        {
            if (strcmp(p_temp->name, argv[1]) == 0)
            {
                gm_cli_put_str("command:");
                gm_cli_put_str(p_temp->name);
                if (p_temp->link != NULL)
                {
                    gm_cli_put_str(" -> ");
                    p_temp = p_temp->link;
                    gm_cli_put_str(p_temp->name);
                }
                gm_cli_put_str("\r\n  usage:");
                gm_cli_put_str(p_temp->usage);
                gm_cli_put_str("\r\n");
                found_flag = 1;
                break;
            }
            p_temp = gm_cli_get_next_cmd((const int*)p_temp);
        }
        if (found_flag == 0)
        {
            gm_cli_put_str("Not found command \"");
            gm_cli_put_str(argv[1]);
            gm_cli_put_str("\"\r\n");
        }
    }
    else
    {
        gm_cli_put_str("Too many args! Only support less then 2 args\r\n");
    }

    return 0;
}
/* 导出help命令 */
GM_CLI_CMD_EXPORT(help,
                  "help [cmd] -- list the command and usage",
                  gm_cli_internal_cmd_help);
/* 设置help的别名'?' */
GM_CLI_CMD_ALIAS(help, "?");

/* 内部命令-test */
static int gm_cli_internal_cmd_test(int argc, char* argv[])
{
    gm_cli_printf("cmd  -> %s\r\n", argv[0]);
    for (int i = 1; i < argc; i++)
    {
        gm_cli_printf("arg%d -> %s\r\n", i, argv[i]);
    }
    return 0;
}
/* 导出test命令 */
GM_CLI_CMD_EXPORT(test,
                  "test [args] -- test the cli",
                  gm_cli_internal_cmd_test);
