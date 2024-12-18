#include "Encoder.h"
#include "spi.h"
#include "math.h"

/********������*********/
uint32_t LastEncoderAngle = 0,LastQuad = 0;
int32_t  Rotation = 0;
/***********************/
double EncoderAngle = 0;        // �Ƕ�
float Speed = 0;                // �ٶ�
float SinThe = 0, CosThe = 0;   // ��Ƕ�sin��cosֵ
float DeAngle = 0;			    // ��ʼ����Ƕ����
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
	// ��ȡ������ԭʼ����

	LastEncoderAngle = (uint8_t)(FilterPara * LastEncoderAngle + (1 - FilterPara) * Encoder_Buf[1]);
	Angle = ((Encoder_Buf[0] << 8 | LastEncoderAngle) & 0x7fff) * 0.010986328f;

	Quadrant = Angle > 180 ? (Angle > 270 ? 4 : 3) : (Angle > 90 ? 2 : 1);
	// �����ж�

	if (LastQuad == 1 && Quadrant == 4)
		Rotation--;
	if (LastQuad == 4 && Quadrant == 1)
		Rotation++;
	// ��Ȧ
	LastQuad = Quadrant;
	return (Angle + Rotation * 360);
	// �����ۼƽǶ�
} // TLE5012B������ͨ�����޼�Ȧ

void GetAngle()
{
	float Theta = TLE5012_GetAngle() - DeAngle;
	float FilterPara = 0.9;

	Speed = FilterPara * Speed + (1 - FilterPara) * (Theta - EncoderAngle) * 1000.0f; // �����ٶ�ʹ���˼򵥵ĵ�ͨ�˲�����Ч��һ��

	EncoderAngle = Theta;
	Theta *= (PI / 25.7142857f); // ��Ϊ1/25�ľ�ȷ��̫�ͣ���������û��ʹ�ó˷�
	SinThe = sinf(Theta);
	CosThe = cosf(Theta);

	printf("%f\n",EncoderAngle);
} // ��õ�ǶȲ�������ٶȣ���sin��cosֵ����������