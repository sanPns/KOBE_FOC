#include "OLED_DRV.h"

u8g2_t *u8g2;
SPI_HandleTypeDef *OLED_hspi;

uint8_t u8x8_stm32_gpio_and_delay(U8X8_UNUSED u8x8_t *u8x8,
								  U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int,
								  U8X8_UNUSED void *arg_ptr)
{
	switch (msg)
	{
	case U8X8_MSG_GPIO_AND_DELAY_INIT:
		HAL_Delay(1);
		break;
	case U8X8_MSG_DELAY_MILLI:
		HAL_Delay(arg_int);
		break;
	case U8X8_MSG_GPIO_CS:
		break;
	case U8X8_MSG_GPIO_DC:
		HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, (GPIO_PinState)arg_int);
		break;
	case U8X8_MSG_GPIO_RESET:
		HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, (GPIO_PinState)arg_int);
		break;
	}
	return 1;
}
uint8_t u8x8_byte_4wire_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int,
							   void *arg_ptr)
{
	switch (msg)
	{
	case U8X8_MSG_BYTE_SEND:
		HAL_SPI_Transmit(OLED_hspi, (uint8_t *)arg_ptr, arg_int, 0xffff);
		// HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)arg_ptr, arg_int);
		// while(hspi1.TxXferCount);
		break;
	case U8X8_MSG_BYTE_INIT:
		break;
	case U8X8_MSG_BYTE_SET_DC:
		u8x8_gpio_SetDC(u8x8, arg_int);
		break;
	case U8X8_MSG_BYTE_START_TRANSFER:
		// HAL_GPIO_WritePin(OLED_CS_GPIO_Port,OLED_CS_Pin, 0);
		break;
	case U8X8_MSG_BYTE_END_TRANSFER:
		// HAL_GPIO_WritePin(OLED_CS_GPIO_Port,OLED_CS_Pin, 1);
		break;
	default:
		return 0;
	}
	return 1;
}

int OLED_printf(int line,int len, const char *format, ...)
{
	va_list ap;
	int ret = -1;
	char str[len];
	va_start(ap, format);
	vsprintf(str, format, ap);
	va_end(ap);
	u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
	u8g2_DrawStr(u8g2, 0, line*13, str);
	return ret;
}

void OLED_Init(SPI_HandleTypeDef *hspi,u8g2_t *u8g2t)
{
	OLED_hspi = hspi;
	u8g2 = u8g2t;
	u8g2_Setup_ssd1306_128x64_noname_f(u8g2, U8G2_R1, u8x8_byte_4wire_hw_spi, u8x8_stm32_gpio_and_delay);
	u8g2_InitDisplay(u8g2);
	u8g2_SetPowerSave(u8g2, 0);
}