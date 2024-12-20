/**
 * FOC驱动部分代码，代码内容不多，变量命名比较随意……
 */
#include "Driver.h"
#include "tim.h"
#include "Encoder.h"
#include "usart.h"
#include <stdio.h>
#include "adc.h"
#include "main.h"

/************************************************************************/
float Ia = 0.0f, Ib = 0.0f; // 克拉克变换电流值
float Iq = 0.0f, Id = 0.0f; // 帕克变换电流值

float Vq = 0.0f, Vd = 0.0f, Udc = 12; // q、d轴电压控制值，电源电压
float Ua = 0, Ub = 0;				  // 帕克逆变换电压值

extern uint16_t Adcx[2]; // ADC电流采样值
float Cur_val[3] = {0};	 // ADC电流实际值

float CurIN = 0.0f; // 目标力矩
float SpdIN = 0.0f; // 目标速度
float LocIN = 0.0f; // 目标位置
/************************************************************************/
// PWM输出
void SetPwm(int A, int B, int C)
{
	__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, B);
	__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_2, A);
	__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_3, C);
}

/************************************************************************/
// 克拉克变换
void Clark()
{
	Ia = Cur_val[0];
	Ib = (Cur_val[2] - Cur_val[1]) / sqrt3;
}

// 帕克变换
void Park()
{
	Id = Ia * CosThe + Ib * SinThe;
	Iq = Ib * CosThe - Ia * SinThe;
}

// 逆帕克变换
void Repark()
{
	Ua = -Vq * SinThe + Vd * CosThe;
	Ub = Vq * CosThe + Vd * SinThe;
}

/**
 * SVPWMM
 * 这一段的计算方式是高中自习课摸鱼推出来的,注释也没怎么写，现在我自己也不记得各个变量的意义了
 * 如果有兴趣可以对着推一下，我设定这几个中间变量应该是为了减少计算量
 */
void SVPWM()
{
	float TL = 500, TD = 0, TS = 0;
	float K = 62.5f, Uba = Ub / sqrt3;
	float Ta = 0, Tb = 0, Tc = 0;

	int Ph = Uba > 0 ? (abs(Uba) > abs(Ua) ? 1 : (Ua > 0 ? 0 : 2)) : (abs(Uba) > abs(Ua) ? 4 : (Ua > 0 ? 5 : 3));
	// 扇区判断

	switch (Ph)
	{
	case 0:
		TD = (Ua - Uba) * K;
		TS = 2 * Uba * K;
		Ta = (TL - TD - TS) * 0.5f;
		Tb = (TL + TD - TS) * 0.5f;
		Tc = (TL + TD + TS) * 0.5f;
		break;
	case 1:
		TD = (Uba - Ua) * K;
		TS = (Uba + Ua) * K;
		Ta = (TL + TD - TS) * 0.5f;
		Tb = (TL - TD - TS) * 0.5f;
		Tc = (TL + TD + TS) * 0.5f;
		break;
	case 2:
		TD = 2 * Uba * K;
		TS = (-Uba - Ua) * K;
		Ta = (TL + TD + TS) * 0.5f;
		Tb = (TL - TD - TS) * 0.5f;
		Tc = (TL + TD - TS) * 0.5f;
		break;
	case 3:
		TD = -2 * Uba * K;
		TS = (Uba - Ua) * K;
		Ta = (TL + TD + TS) * 0.5f;
		Tb = (TL + TD - TS) * 0.5f;
		Tc = (TL - TD - TS) * 0.5f;
		break;
	case 4:
		TD = (-Uba - Ua) * K;
		TS = (Ua - Uba) * K;
		Ta = (TL + TD - TS) * 0.5f;
		Tb = (TL + TD + TS) * 0.5f;
		Tc = (TL - TD - TS) * 0.5f;
		break;
	case 5:
		TD = (Uba + Ua) * K;
		TS = -2 * Uba * K;
		Ta = (TL - TD - TS) * 0.5f;
		Tb = (TL + TD + TS) * 0.5f;
		Tc = (TL + TD - TS) * 0.5f;
		break;
	}
	SetPwm((int)Ta, (int)Tb, (int)Tc);
}

/************************************************************************/

/**
 * 这里单独说明一下，三个闭环都是独立的，分别操控Vq，所以同时只能使用一个闭环
 * 至于原因，一个是电流开环下的速度位置环效果已经足够好，
 */

// 电流闭环
float Cur_Q_EI = 0, Cur_D_EI = 0; // 力矩误差积分
void Cur_PI()
{
	float Cur_Q_EN = (CurIN - Iq);
	float Cur_D_EN = (0 - Id);
	float Kp_q = 0.0255f, Ki_q = 0.006f;
	float Volq = (Kp_q * Cur_Q_EN + Ki_q * Cur_Q_EI);

	Cur_Q_EI += Cur_Q_EN;
	if (abs(Cur_Q_EI) > 46.0f)
		Cur_Q_EI = (Cur_Q_EI > 0 ? 1 : -1) * 46.0f;
	// 积分限幅

	Vq = Volq;

	float Kp_d = 0.086f, Ki_d = 0.105f;
	float Vold = (Kp_d * Cur_D_EN + Ki_d * Cur_D_EI);

	Cur_D_EI += Cur_D_EN;
	if (abs(Cur_D_EI) > 20.0f)
		Cur_D_EI = (Cur_D_EI > 0 ? 1 : -1) * 20.0f;
	// 积分限幅

	Vd = Vold;
}

// 速度闭环
float Spd_EI = 0; // 速度误差积分
void Spd_PI()
{
	float Spd_EN = 0.001f * (SpdIN - Speed);
	float Kp = 0.35f, Ki = 0.0548f;
	float spd = (Kp * Spd_EN + Ki * Spd_EI);

	if (abs(Spd_EI) > 30.0f)
		Spd_EI = (Spd_EI > 0 ? 1.0f : -1.0f) * 30.0f;
	// 积分限幅

	spd = abs(spd) > 2.0f ? (spd > 0 ? 1.0f : -1.0f) * 2.0f : spd;
	// 整体限幅

	Vq = spd;

	Spd_EI += Spd_EN;
}

// 位置闭环
float Loc_EL = 0, Loc_EI = 0; // 分别是上次位置误差和位置误差积分
void Loc_PID()
{
	float Loc_EN = (LocIN - EncoderAngle);
	float Kp = 0.03f, Ki = 0.000015f, Kd = 0.0004f;
	float loc = (Kp * Loc_EN + Ki * Loc_EI - Kd * (Loc_EN - Loc_EL));

	Loc_EI += Loc_EN;

	if (abs(Loc_EI) > 20.0f)
		Loc_EI = (Loc_EI > 0 ? 1.0f : -1.0f) * 20.0f;
	// 积分限幅

	loc = abs(loc) > 2.0f ? (loc > 0 ? 1.0f : -1.0f) * 2.0f : loc;
	// 整体限幅

	Vq = loc;
	// printf("%.2f,%.2f\n", Vq,EncoderAngle);

	Loc_EL = Loc_EN;
}

/************************************************************************/

void FOC_Cycle()
{
	GetAngle();
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)Adcx, 2);
	Cur_Cycle();
	Loc_PID();
	// Spd_PI();
} // 电流变换

void Cur_Cycle()
{
	Clark();
	Park();
	// Cur_PI();
	Repark();
	SVPWM();
} // 电流变换

void AngInit()
{
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	SetPwm(0, 60, 60);
	HAL_Delay(300);
	DeAngle = TLE5012_GetAngle();
	DeAngle = TLE5012_GetAngle();
	SVPWM();
} // 初始化电角度零点