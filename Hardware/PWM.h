#ifndef __PWM_H
#define __PWM_H

#include "stm32f10x.h"

void PWM_LeftMotor_Init(void);
void PWM_RightMotor_Init(void);
void PWM_SetLeftMotorCompare(uint16_t Compare);
void PWM_SetRightMotorCompare(uint16_t Compare);

#endif
