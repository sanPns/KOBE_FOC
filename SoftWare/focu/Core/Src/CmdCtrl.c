/**
 * file CmdCtrl.c
 * version 0.1
 * date 2024-08-24
 * Encoding GB2312
 *  ���ڽ�����λ��ָ��ĺ��Ĵ���
 *  ��λ��ָ����ASCII���룬ÿ��ָ����\n��β
 *  ֧�ֽ�������Ϊ��λ���ַ�ָ���Сд������
 *  �ַ�ָ���ɽ� ":[����]" ��ʽ�����ݣ����ݻ����ַ�������ʽ����ص�����
 *  �ص�����д��CmdOrder.c�У�ͨ��init�������ص�ָ�����
 */

#include "CmdCtrl.h"

UART_HandleTypeDef *huart;
/*ָ���ȡ����*/
char buf[100];
int bufpi = 0;

char bufs[100];
int bufpis = 0;
/************/

/*ָ���*/
int ComListLeng = 0;
Com_Typedef CmdList[20];
/*******/

/**
 * @brief ���ڳ�ʼ��uart���裬��ָ��һ��uart�ӿ�
 *
 * @param huaring ָ����uart����
 */
void CmdCtrl_Init(UART_HandleTypeDef *huaring)
{
    huart = huaring;
    HAL_UART_Receive_IT(huart, (uint8_t *)buf, 1);
}

/**
 * @brief ���ڽ�ָ���ص���������ָ���
 *
 * @param ComString ָ��ֵ��������λ����Ҫ��ĩβ��\0������
 * @param Fun ָ���ص��������Ժ���ָ����ʽ����
 */
void SetCmdTypedef(const char *ComString, void (*Fun)(const char *))
{
    strcpy(CmdList[ComListLeng].ComStr, ComString);
    CmdList[ComListLeng].CallBackFun = Fun;
    ComListLeng++;
}

/**
 * @brief ���ڴ����ȡ����ָ��
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
 * @brief ���ڶ�ȡ���յ��ĵ��ֽ�����
 *
 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *INhuart)
{
    if (INhuart == huart)
    {
        if (buf[bufpi] == '\n')
        {
            Log_Del(); // ������������յ����ַ���
            memset(buf, 0, sizeof(buf));
            bufpi = -1;
        }
        bufpi++;

        HAL_UART_Receive_IT(huart, (uint8_t *)(buf + bufpi), 1);
    }
}