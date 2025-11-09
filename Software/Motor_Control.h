#ifndef __MOTOR_CONTROL_H
#define __MOTOR_CONTROL_H

#include "stm32f10x.h"
#include "motor_data.h"

// 函数声明
void MotorControl_Init(void);
void MotorControl_Update(void);
void MotorControl_ResetIntegral(void);

#endif
