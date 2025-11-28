#ifndef __COMMUNICATION_H
#define __COMMUNICATION_H

#include "stm32f10x.h"

// 函数声明
void Serial_Command_Init(void);
void Serial_Command_RxHandler(uint8_t data);
uint8_t Serial_Command_Available(void);
char* Serial_Command_GetCommand(void);
void Serial_Command_ClearFlag(void);
void Serial_Command_Process(void);

#endif
