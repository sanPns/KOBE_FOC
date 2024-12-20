/**
 * version 0.1
 * date 2024-08-24
 * Encoding GB2312
 *  ��λ��ָ����ASCII���룬ÿ��ָ����\n��β
 *  ֧�ֽ�������Ϊ��λ���ַ�ָ���Сд������
 *  �ַ�ָ���ɽ� ":[����]" ��ʽ�����ݣ����ݻ����ַ�������ʽ����ص�����
 *  �ص�����д��CmdOrder.c�У�ͨ��init�������ص�ָ�����
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