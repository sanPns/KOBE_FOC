/**
 * FOC�������ִ��룬�������ݲ��࣬���������Ƚ����⡭��
 */
#include "Driver.h"
#include "tim.h"
#include "Encoder.h"
#include "usart.h"
#include <stdio.h>
#include "adc.h"
#include "main.h"

/************************************************************************/
float Ia = 0.0f, Ib = 0.0f; // �����˱任����ֵ
float Iq = 0.0f, Id = 0.0f; // ���˱任����ֵ

float Vq = 0.0f, Vd = 0.0f, Udc = 12; // q��d���ѹ����ֵ����Դ��ѹ
float Ua = 0, Ub = 0;				  // ������任��ѹֵ

extern uint16_t Adcx[2]; // ADC��������ֵ
float Cur_val[3] = {0};	 // ADC����ʵ��ֵ

float CurIN = 0.0f; // Ŀ������
float SpdIN = 0.0f; // Ŀ���ٶ�
float LocIN = 0.0f; // Ŀ��λ��
/************************************************************************/
// PWM���
void SetPwm(int A, int B, int C)
{
	__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, B);
	__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_2, A);
	__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_3, C);
}

/************************************************************************/
// �����˱任
void Clark()
{
	Ia = Cur_val[0];
	Ib = (Cur_val[2] - Cur_val[1]) / sqrt3;
}

// ���˱任
void Park()
{
	Id = Ia * CosThe + Ib * SinThe;
	Iq = Ib * CosThe - Ia * SinThe;
}

// �����˱任
void Repark()
{
	Ua = -Vq * SinThe + Vd * CosThe;
	Ub = Vq * CosThe + Vd * SinThe;
}

/**
 * SVPWMM
 * ��һ�εļ��㷽ʽ�Ǹ�����ϰ�������Ƴ�����,ע��Ҳû��ôд���������Լ�Ҳ���ǵø���������������
 * �������Ȥ���Զ�����һ�£����趨�⼸���м����Ӧ����Ϊ�˼��ټ�����
 */
void SVPWM()
{
	float TL = 500, TD = 0, TS = 0;
	float K = 62.5f, Uba = Ub / sqrt3;
	float Ta = 0, Tb = 0, Tc = 0;

	int Ph = Uba > 0 ? (abs(Uba) > abs(Ua) ? 1 : (Ua > 0 ? 0 : 2)) : (abs(Uba) > abs(Ua) ? 4 : (Ua > 0 ? 5 : 3));
	// �����ж�

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
 * ���ﵥ��˵��һ�£������ջ����Ƕ����ģ��ֱ�ٿ�Vq������ͬʱֻ��ʹ��һ���ջ�
 * ����ԭ��һ���ǵ��������µ��ٶ�λ�û�Ч���Ѿ��㹻�ã�
 */

// �����ջ�
float Cur_Q_EI = 0, Cur_D_EI = 0; // ����������
void Cur_PI()
{
	float Cur_Q_EN = (CurIN - Iq);
	float Cur_D_EN = (0 - Id);
	float Kp_q = 0.0255f, Ki_q = 0.006f;
	float Volq = (Kp_q * Cur_Q_EN + Ki_q * Cur_Q_EI);

	Cur_Q_EI += Cur_Q_EN;
	if (abs(Cur_Q_EI) > 46.0f)
		Cur_Q_EI = (Cur_Q_EI > 0 ? 1 : -1) * 46.0f;
	// �����޷�

	Vq = Volq;

	float Kp_d = 0.086f, Ki_d = 0.105f;
	float Vold = (Kp_d * Cur_D_EN + Ki_d * Cur_D_EI);

	Cur_D_EI += Cur_D_EN;
	if (abs(Cur_D_EI) > 20.0f)
		Cur_D_EI = (Cur_D_EI > 0 ? 1 : -1) * 20.0f;
	// �����޷�

	Vd = Vold;
}

// �ٶȱջ�
float Spd_EI = 0; // �ٶ�������
void Spd_PI()
{
	float Spd_EN = 0.001f * (SpdIN - Speed);
	float Kp = 0.35f, Ki = 0.0548f;
	float spd = (Kp * Spd_EN + Ki * Spd_EI);

	if (abs(Spd_EI) > 30.0f)
		Spd_EI = (Spd_EI > 0 ? 1.0f : -1.0f) * 30.0f;
	// �����޷�

	spd = abs(spd) > 2.0f ? (spd > 0 ? 1.0f : -1.0f) * 2.0f : spd;
	// �����޷�

	Vq = spd;

	Spd_EI += Spd_EN;
}

// λ�ñջ�
float Loc_EL = 0, Loc_EI = 0; // �ֱ����ϴ�λ������λ��������
void Loc_PID()
{
	float Loc_EN = (LocIN - EncoderAngle);
	float Kp = 0.03f, Ki = 0.000015f, Kd = 0.0004f;
	float loc = (Kp * Loc_EN + Ki * Loc_EI - Kd * (Loc_EN - Loc_EL));

	Loc_EI += Loc_EN;

	if (abs(Loc_EI) > 20.0f)
		Loc_EI = (Loc_EI > 0 ? 1.0f : -1.0f) * 20.0f;
	// �����޷�

	loc = abs(loc) > 2.0f ? (loc > 0 ? 1.0f : -1.0f) * 2.0f : loc;
	// �����޷�

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
} // �����任

void Cur_Cycle()
{
	Clark();
	Park();
	// Cur_PI();
	Repark();
	SVPWM();
} // �����任

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
} // ��ʼ����Ƕ����