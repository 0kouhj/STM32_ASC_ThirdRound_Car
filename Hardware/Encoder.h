// Encoder.h
#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f10x.h"

// 函数声明
void Encoder_Init(void);
int16_t Encoder_GetLeft(void);
int16_t Encoder_GetRight(void);
int16_t Encoder_ReadLeft(void);
int16_t Encoder_ReadRight(void);
void Encoder_Reset(void);
#endif
