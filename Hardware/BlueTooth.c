#include "stm32f10x_gpio.h"
#include "Sensor_Data.h"
Joystick_Data joystick_data = {
	.x = 0,
	.y = 0,
	.sensitivity = 1.0f
};
/**
  * 函    数：初始化蓝牙状态检测引脚
  */
void Bluetooth_StatePin_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 开启GPIOB时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    // 配置PB9为输入模式（蓝牙STATE引脚）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

uint8_t Bluetooth_GetHardwareState(void)
{
    if (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9) == 1)
	{
		return 1;
	}
	else return 0;
}
