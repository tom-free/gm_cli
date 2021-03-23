# 通用嵌入式跨平台命令行接口（GM_CLI）

```cmd

-------------------------------------------------
[GM CLI] > help test
-------------------------------------------------

-------------------------------------------------

-------------------------------------------------

```

## 特性

> 1. 带有命令导出功能，修改添加命令无需修改命令行相关代码，只需要导出新的命令即可</br>
> 2. 带有历史记录功能，用户可以配置记录条数</br>
> 3. 带有命令自动补全功能，输入时更加的方便快捷</br>
> 4. 代码占用少，执行效率高，无其他模块依赖</br>
> 5. 采用回调机制，用户只需要集中精力在命令的实现上，无需自己解析命令</br>
> 6. 带有系统默认命令，可以快速的查询和测试CLI系统</br>
> 7. 字符的发送采用注册机制，用户可以进行重定向和文件操作，完全自定义</br>
> 8. 字符的接收完全用户决定，可以进行文件操作或流读取，可以轻松实现脚本解释器的功能</br>

## 计划

> 1. 添加参数的自动补全功能
> 2. 添加模仿Linux命令的可选参数和复合参数等功能
> 3. 添加函数执行指令，便于代码调试

## 使用说明

1. 准备一个工程，带有字符输入输出功能或模拟数据输入输出功能即可
1. 下载此代码到工程目录，在需要放代码的位置，右键打开Git Bash

HTTPS下载输入

```bash
git clone https://gitee.com/tomfree_opensource/cli_test.git
```

SSH下载输入

```bash
git clone git@gitee.com:tomfree_opensource/cli_test.git
```

3. 添加 `gm_cli.c` 文件到工程，具体添加方式不同编译器不同

4. 添加 `gm_cli.h` 所在的目录到编译头文件包含目录，具体添加方式不同编译器不同

5. 编译代码，解决一些因编译器不同产生的错误或警告，如有没法解决的错误或警告，请查询编译器手册或发送邮件给我一起探讨学习并解决（1124339397@qq.com）

6. main函数while之前或主任务运行之前添加如下初始化代码（your_out_char_cb需要填入自己的字符输出驱动；Your Prompt需要填入自定义的提示符，不调用此函数系统采用默认的提示符）

```C
/* 初始化 */
GM_CLI_Init();
/* 注册输出驱动 */
GM_CLI_RegOutCharCallBack((GM_CLI_OUT_CHAR_CB)your_out_char_cb);
/* 设置提示符 */
GM_CLI_SetCommandPrompt("[Your Prompt] > ");
/* 启动CLI */
GM_CLI_Start();
```

7. 初始化后先采用如下代码测试CLI是否正常

```C
const char str[] = "test 1 2 3\n";
unsigned int len = (unsigned int)strlen(str);
unsigned int i;
for (i = 0; i < len; i++)
{
    GM_CLI_ParseOneChar(str[i]);
}
```

如果输出如下表示CLI正常

```bash
cmd  -> test
arg1 -> 1
arg2 -> 2
arg3 -> 3
```

7. main函数while中或主任务或建立一个CLI任务，执行以下代码（_kbhit替换成自己的字符输入检测函数，_getch替换成自己的子符读取函数）

```C
/* 键盘检测或数据队列查询等 */
if (_kbhit())
{
    /* 读取字符 */
    ch = _getch();
    /* 解析字符 */
    GM_CLI_ParseOneChar((char)ch);
}
```

8. 编译代码，下载或进入调试，打开相应数据输入终端，按回车键查看是否有提示符输出，可以输入`test`或`help`指令检测CLI是否正常

## 添加命令

1. 命名回调函数格式如下：

```C
int command_callback(int argc, char* argv[]);
```

argc - 表示输入的命令行命令和参数的总个数，含有命令本身，所以此参数最小值为1

argv - 表示命令参数的内容，采用指针存放，类型都是`char`型的指针即字符串地址

2. 实现回调函数后只需要执行如下的导出命令即可

```C
GM_CLI_CMD_EXPORT(command_name, "command_usage_string", command_callback);
```

command_name - 命令的名字，这个名字就是命令的识别名，输入搜索的就是这个名字

command_usage_string - 命令的使用说明字符串，用于`help`命令给用户提示使用说明

command_callback - 命令回调函数

3. 编译代码，下载调试即可使用此命令，命令详细使用说明可输入`help command_name`查看，也可直接使用`help`查看系统当前支持的所有命令

## 特殊说明

此代码还未完成测试，现在邀请大家进行测试和评价，大家有什么建议或疑问请邮件或直接在仓库进行留言，谢谢大家的参与和代码的共同维护，也希望大家将自己的优秀的计划加入代码中并发出pull request

