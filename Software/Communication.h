#include "stm32f10x.h"                  // Device header
#include <stdint.h>
#ifndef __COMMUNICATION_H
#define __COMMUNICATION_H

// 函数声明
void Serial_Command_Init(void);
void Serial_Command_RxHandler(uint8_t data);
uint8_t Serial_Command_Available(void);
char* Serial_Command_GetCommand(void);
void Serial_Command_ClearFlag(void);
void Serial_Command_Process(void);

// 需要外部实现的串口发送函数
extern void Serial_SendString(char *String);

#endif
