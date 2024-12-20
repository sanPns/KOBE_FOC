/**
 * version 0.1
 * date 2024-08-24
 * Encoding GB2312
 *  上位机指令以ASCII编码，每条指令以\n结尾
 *  支持解析长度为三位的字符指令，大小写不敏感
 *  字符指令后可接 ":[数据]" 格式的数据，数据会以字符串的形式传入回调函数
 *  回调函数写在CmdOrder.c中，通过init函数加载到指令表中
 */

#include "CmdOrder.h"

/***********/
#include "Driver.h"
/***********/
int atoi(const char *Value)
{
	int val = 0;
	int ch = 1;
	if (*Value == '-')
		ch = -1, Value++;
	while (1)
	{
		val *= 10;
		val += *Value - '0';
		Value++;
		if (*(Value + 1) == '\0')
			break;
	}
	return val * ch;
}

void loc(const char *Value)
{
	int VAL = atoi(Value);
	LocIN = (float)VAL;
	printf("%d\n", VAL);
}

void spd(const char *Value)
{
	int VAL = atoi(Value);
	SpdIN = (float)VAL;
	printf("%d\n", VAL);
}

/***********/

void CmdOrder_Init(UART_HandleTypeDef *huaring)
{
	CmdCtrl_Init(huaring);
	SetCmdTypedef("loc\0", &loc);
	SetCmdTypedef("spd\0", &spd);
}