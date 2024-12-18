#ifndef _ENCODER_H_
#define _ENCODER_H_

extern float SinThe, CosThe; // 电角度sin，cos值
extern float DeAngle;        // 初始化电角度零点
extern double EncoderAngle;
extern float Speed;

float TLE5012_GetAngle();
void GetAngle();

#endif