#include "CmdOrder.h"

/***********/
// #include<stdlib.h>
#include "Driver.h"
/***********/

int atoi(const char *Value)
{
	int val = 0;
	while (1)
	{
		val *= 10;
		val += *Value - '0';
		Value++;
		if (*(Value+1) == '\0')
			break;
	}
	return val;
}

void loc(const char *Value)
{
	int VAL = atoi(Value);
	LocIN = (float)VAL;
	printf("%d\n",VAL);
}

void spd(const char *Value)
{
	int VAL = atoi(Value);
	LocIN = (float)VAL;
	printf("%d..\n",VAL);
}

/***********/

void CmdOrder_Init(UART_HandleTypeDef *huaring)
{
	CmdCtrl_Init(huaring);
	SetCmdTypedef("loc\0", &loc);
	SetCmdTypedef("spd\0", &spd);
}