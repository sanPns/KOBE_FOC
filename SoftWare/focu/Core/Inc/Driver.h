#ifndef _DRIVER_H_
#define _DRIVER_H_

extern float CurIN; // 目标力矩
extern float SpdIN; // 目标速度
extern float LocIN; // 目标位置
extern float Cur_val[3];

void SetPwm(int A, int B, int C);
void FOC_Cycle();
void Cur_Cycle();
void Spd_PI();
void Loc_PID();
void AngInit();

#endif