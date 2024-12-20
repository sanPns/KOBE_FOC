#ifndef _DRIVER_H_
#define _DRIVER_H_

extern float CurIN; // Ŀ������
extern float SpdIN; // Ŀ���ٶ�
extern float LocIN; // Ŀ��λ��
extern float Cur_val[3];

void SetPwm(int A, int B, int C);
void FOC_Cycle();
void Cur_Cycle();
void Spd_PI();
void Loc_PID();
void AngInit();

#endif