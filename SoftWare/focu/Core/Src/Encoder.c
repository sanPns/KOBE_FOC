#include "Encoder.h"
#include "spi.h"
#include "math.h"

/********编码器*********/
uint32_t LastEncoderAngle = 0,LastQuad = 0;
int32_t  Rotation = 0;
/***********************/
double EncoderAngle = 0;        // 角度
float Speed = 0;                // 速度
float SinThe = 0, CosThe = 0;   // 电角度sin，cos值
float DeAngle = 0;			    // 初始化电角度零点
/***********************/

float TLE5012_GetAngle()
{
	float Angle;
    uint32_t Quadrant = 0;
	uint8_t Encoder_Buf[2] = {0x80, 0x21};
	float FilterPara = 0.82;

	Encoder_CSN(LOW);
	HAL_SPI_Transmit(&hspi2, Encoder_Buf, 2, 0xfff);
	HAL_SPI_Receive(&hspi2, Encoder_Buf, 2, 0xfff);
	Encoder_CSN(HIGH);
	// 读取编码器原始数据

	LastEncoderAngle = (uint8_t)(FilterPara * LastEncoderAngle + (1 - FilterPara) * Encoder_Buf[1]);
	Angle = ((Encoder_Buf[0] << 8 | LastEncoderAngle) & 0x7fff) * 0.010986328f;

	Quadrant = Angle > 180 ? (Angle > 270 ? 4 : 3) : (Angle > 90 ? 2 : 1);
	// 象限判断

	if (LastQuad == 1 && Quadrant == 4)
		Rotation--;
	if (LastQuad == 4 && Quadrant == 1)
		Rotation++;
	// 记圈
	LastQuad = Quadrant;
	return (Angle + Rotation * 360);
	// 返回累计角度
} // TLE5012B驱动，通过象限记圈

void GetAngle()
{
	float Theta = TLE5012_GetAngle() - DeAngle;
	float FilterPara = 0.9;

	Speed = FilterPara * Speed + (1 - FilterPara) * (Theta - EncoderAngle) * 1000.0f; // 计算速度使用了简单的低通滤波器，效果一般

	EncoderAngle = Theta;
	Theta *= (PI / 25.7142857f); // 因为1/25的精确度太低，所以这里没有使用乘法
	SinThe = sinf(Theta);
	CosThe = cosf(Theta);

	printf("%f\n",EncoderAngle);
} // 获得电角度并计算出速度，及sin，cos值减少运算量