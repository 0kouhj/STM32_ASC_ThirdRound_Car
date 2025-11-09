#include "stdint.h"
#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32f10x.h"
#include <stdio.h>
#include <stdarg.h>

// 原有的变量声明
extern char Serial_RxPacket[100];
extern uint8_t Serial_RxFlag;

// 新增的简化接收变量声明
extern volatile uint8_t usart_rx_buffer[100];
extern volatile uint8_t usart_rx_index;
extern volatile uint8_t usart_rx_ready;

// 原有的函数声明
void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);
void Serial_SendNumber(uint32_t Number, uint8_t Length);
int fputc(int ch, FILE *f);
void Serial_Printf(char *format, ...);

#endif
