#ifndef _ENCODER_H_
#define _ENCODER_H_

extern float SinThe, CosThe; // ��Ƕ�sin��cosֵ
extern float DeAngle;        // ��ʼ����Ƕ����
extern double EncoderAngle;
extern float Speed;

float TLE5012_GetAngle();
void GetAngle();

#endif