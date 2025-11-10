#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"

// LED状态定义
#define LED_ON  0
#define LED_OFF 1

// 根据您的开发板选择LED引脚
// 这里以常见的PC13为例，请根据实际修改

// LED引脚定义
#define LED_GPIO_PORT    GPIOC
#define LED_GPIO_PIN     GPIO_Pin_13
#define LED_GPIO_CLK     RCC_APB2Periph_GPIOC

// 函数声明
void LED_Init(void);
void LED_On(void);
void LED_Off(void);
void LED_Toggle(void);

#endif
