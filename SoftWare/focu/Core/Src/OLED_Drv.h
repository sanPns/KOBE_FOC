#ifndef __OLED_DRV_H__
#define __OLED_DRV_H__

#include "main.h"
#include "spi.h"
#include "dma.h"
#include<stdio.h>
#include<stdarg.h>
#include "./u8g2/u8g2.h"

void OLED_Init(SPI_HandleTypeDef *hspi,u8g2_t *u8g2t);
int OLED_printf(int line,int len, const char *format, ...);
void OLED_circle(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rad, uint8_t option);

#endif
















