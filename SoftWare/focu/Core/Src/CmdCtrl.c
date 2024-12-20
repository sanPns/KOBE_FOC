/**
 * file CmdCtrl.c
 * version 0.1
 * date 2024-08-24
 * Encoding GB2312
 *  用于解析上位机指令的核心代码
 *  上位机指令以ASCII编码，每条指令以\n结尾
 *  支持解析长度为三位的字符指令，大小写不敏感
 *  字符指令后可接 ":[数据]" 格式的数据，数据会以字符串的形式传入回调函数
 *  回调函数写在CmdOrder.c中，通过init函数加载到指令表中
 */

#include "CmdCtrl.h"

UART_HandleTypeDef *huart;
/*指令读取缓存*/
char buf[100];
int bufpi = 0;

char bufs[100];
int bufpis = 0;
/************/

/*指令表*/
int ComListLeng = 0;
Com_Typedef CmdList[20];
/*******/

/**
 * @brief 用于初始化uart外设，可指定一个uart接口
 *
 * @param huaring 指定的uart外设
 */
void CmdCtrl_Init(UART_HandleTypeDef *huaring)
{
    huart = huaring;
    HAL_UART_Receive_IT(huart, (uint8_t *)buf, 1);
}

/**
 * @brief 用于将指令及其回调函数载入指令表
 *
 * @param ComString 指令值，长度三位，需要在末尾加\0结束符
 * @param Fun 指定回调函数，以函数指针形式传入
 */
void SetCmdTypedef(const char *ComString, void (*Fun)(const char *))
{
    strcpy(CmdList[ComListLeng].ComStr, ComString);
    CmdList[ComListLeng].CallBackFun = Fun;
    ComListLeng++;
}

/**
 * @brief 用于处理读取到的指令
 *
 */
void Log_Del()
{
    char Command[3] = "   ";
    int ComToDrv;

    for (int i = 0; i < 3; i++)
        Command[i] = buf[i] < 97 ? buf[i] + 32 : buf[i];
    for (ComToDrv = 0; ComToDrv <= ComListLeng; ComToDrv++)
    {
        if (!strcmp(Command, CmdList[ComToDrv].ComStr))
            break;
    }
    if (ComToDrv == ComListLeng)
        return;
    CmdList[ComToDrv].CallBackFun((const char *)(buf + 4));
}

/**
 * @brief 用于读取接收到的单字节数据
 *
 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *INhuart)
{
    if (INhuart == huart)
    {
        if (buf[bufpi] == '\n')
        {
            Log_Del(); // 这个函数处理收到的字符串
            memset(buf, 0, sizeof(buf));
            bufpi = -1;
        }
        bufpi++;

        HAL_UART_Receive_IT(huart, (uint8_t *)(buf + bufpi), 1);
    }
}