/*******************************************************************************
** 文件名称：main.c
** 文件作用：主函数文件
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
#include "stdio.h"
#include "conio.h"
#include "windows.h"



/*******************************************************************************
** 函数名称：main
** 函数作用：主函数
** 输入参数：argc - 参数个数，argv - 参数表
** 输出参数：执行结果
** 使用范例：main();
** 函数备注：
*******************************************************************************/
int main(int argc, char* argv[])
{
    int ch;

    /* 初始化 */
    GM_CLI_Init();
    /* 注册输出驱动 */
    GM_CLI_RegOutCharCallBack((GM_CLI_OUT_CHAR_CB)_putch);
    /* 设置提示符 */
    GM_CLI_SetCommandNotice("[VS CLI Simulator] > ");
    /* 启动CLI */
    GM_CLI_Start();

    for (;;)
    {
        /* 键盘检测 */
        if (_kbhit())
        {
            ch = _getch();
            GM_CLI_ParseOneChar((char)ch);
        }
    }

    return 0;
}
