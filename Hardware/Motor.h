#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f10x.h"

// 函数声明
void Motor_Init(void);
void Motor_SetSpeed_Left(int8_t Speed);
void Motor_SetSpeed_Right(int8_t Speed);
#endif
