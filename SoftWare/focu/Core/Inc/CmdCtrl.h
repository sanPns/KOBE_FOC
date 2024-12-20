#ifndef __CMDCTRL_H
#define __CMDCTRL_H

#include "usart.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef struct
{
    char ComStr[4];
    void (*CallBackFun)(const char *);
} Com_Typedef;

void SetCmdTypedef(const char *ComString, void (*Fun)(const char *));
void CmdCtrl_Init(UART_HandleTypeDef *huaring);

#endif
