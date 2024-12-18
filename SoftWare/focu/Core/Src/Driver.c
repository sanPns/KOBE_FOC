#include "main.h"
#include "Driver.h"
#include "tim.h"
#include "Encoder.h"
#include "usart.h"
#include <stdio.h>

/************************************************************************/
float Ia = 0.0f, Ib = 0.0f; // 克拉克变换电流值
float Iq = 0.0f, Id = 0.0f; // 帕克变换电流值

float Vq = 0.5f, Vd = 0.0f, Udc = 12; // q、d轴电压控制值，电源电压
float Ua = 0, Ub = 0;				  // 帕克逆变换电压值

float Cur_val[3] = {0}; // ADC电流实际值

float CurIN = 0.0f; // 目标力矩
float SpdIN = 0.0f; // 目标速度
float LocIN = 360.0f; // 目标位置
/************************************************************************/
void SetPwm(int A, int B, int C)
{
	__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, B);
	__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_2, A);
	__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_3, C);
} // PWM输出

/************************************************************************/

void Clark()
{
	Ia = Cur_val[0];
	Ib = (Cur_val[2] - Cur_val[1]) / sqrt3;
	
} // 克拉克变换

void Park()
{
	Id = Ia * CosThe + Ib * SinThe;
	Iq = Ib * CosThe - Ia * SinThe;
	//printf("%.2f,%.2f\n",Iq,Id);
} // 帕克变换

void Repark()
{
	Ua = -Vq * SinThe + Vd * CosThe;
	Ub = Vq * CosThe + Vd * SinThe;
} // 逆帕克变换

void SVPWM()
{
	float TL = 500, TD = 0, TS = 0;
	float K = 62.5f, Uba = Ub / sqrt3;
	float Ta = 0, Tb = 0, Tc = 0;
	int Ph = Uba > 0 ? (abs(Uba) > abs(Ua) ? 1 : (Ua > 0 ? 0 : 2)) : (abs(Uba) > abs(Ua) ? 4 : (Ua > 0 ? 5 : 3));
	
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
} // SVPWM

/************************************************************************/

float Cur_Q_EI = 0,Cur_D_EI = 0; // 力矩误差积分

void Cur_PI()
{
	float Cur_Q_EN = (CurIN - Iq);
	float Cur_D_EN = (0 - Id);
	float Kp_q = 0.1f, Ki_q = 0.000f;
	

	float Volq = (Kp_q * Cur_Q_EN + Ki_q * Cur_Q_EI);

	if (abs(Volq) > 1.5f)
	{
		Volq = (Volq > 0 ? 1 : -1) * 1.5f;
	}

	Vq = Volq;
	//Vd = Vold;

} // 电流闭环

float Spd_EI = 0; // 速度误差积分

void Spd_PI()
{
	float Spd_EN = 0.001f * (SpdIN - Speed);
	float Kp = 0.22f, Ki = 0.000007f;
	float cur = (Kp * Spd_EN + Ki * Spd_EI);

	cur = abs(cur) > 1.0f ? (cur > 0 ? 1.0f : -1.0f) * 1.0f : cur;

	//CurIN = cur;
	Vq = cur;

	Spd_EI += Spd_EN;
}
// 速度闭环

float Loc_EL = 0, Loc_EI = 0; // 分别是位置误差n-1和位置误差积分

void Loc_PID()
{
	float Loc_EN = (LocIN - EncoderAngle);
	float Kp = 0.03f, Ki = 0.000015f, Kd = 0.0005f;

	float spd = (Kp * Loc_EN + Ki * Loc_EI - Kd * (Loc_EN - Loc_EL));

	Loc_EI += Loc_EN;

	if (abs(spd) > 1.5f)
	{
		spd = (spd > 0 ? 1 : -1) * 1.5f;
		Loc_EI -= Loc_EN;
	}
	
	//printf("%.2f,%.2f\n", Loc_EN,EncoderAngle);
	Vq = spd;
	Loc_EL = Loc_EN;
}
/************************************************************************/

void Cur_Cycle()
{
	Clark();
	Park();
	//Cur_PI();
	Repark();
	SVPWM();
} // 电流变换

void AngInit()
{
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	SetPwm(0, 60, 60);
	HAL_Delay(100);
	DeAngle = TLE5012_GetAngle();
	DeAngle = TLE5012_GetAngle();
	SVPWM();
} // 初始化电角度零点