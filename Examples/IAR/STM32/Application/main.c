
#include "stdio.h"
#include "gm_cli.h"

int test(int argc, char* argv[])
{
    return 0;
}
GM_CLI_CMD_EXPORT(test, "test", test);

GM_CLI_CMD_EXPORT(help, "test", test);

int main(int argc, char* argv[])
{
    for (;;)
    {

    }
    return 0;
}
