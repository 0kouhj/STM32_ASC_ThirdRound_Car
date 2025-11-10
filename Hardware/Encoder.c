// encoder_fixed.c
#include "stm32f10x.h"
#include "encoder.h"
#include "serial.h"
#include "Sensor_Data.h"
// 编码器计数值
volatile int32_t LeftEncoder_Count = 0;
volatile int32_t RightEncoder_Count = 0;
volatile int32_t LeftEncoder_Speed = 0;
volatile int32_t RightEncoder_Speed = 0;

/**
  * 函    数：编码器初始化 - 左编码器TIM1，右编码器TIM4
  */
void Encoder_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    
    /*---------------- 左编码器 (TIM1, PA8, PA9) ----------------*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA, ENABLE);

    // 配置左编码器GPIO PA8(TIM1_CH1), PA9(TIM1_CH2)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置TIM1时基
    TIM_TimeBaseInitStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 0;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

    // 配置编码器接口
    TIM_EncoderInterfaceConfig(TIM1, TIM_EncoderMode_TI12,TIM_ICPolarity_Falling, TIM_ICPolarity_Falling); 
    
    // 启用TIM1
    TIM_Cmd(TIM1, ENABLE);
    TIM_SetCounter(TIM1, 0);


	/*---------------- 右编码器 (TIM4, PB6, PB7) ----------------*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);  // TIM4在APB1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    // 配置右编码器GPIO PB6(TIM4_CH1), PB7(TIM4_CH2)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 配置TIM4时基
    TIM_TimeBaseInitStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 0;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);

    // 配置编码器接口
    TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12,TIM_ICPolarity_Falling, TIM_ICPolarity_Falling);
    
    // 启用TIM4
    TIM_Cmd(TIM4, ENABLE);
    TIM_SetCounter(TIM4, 0);
    
    //Serial_SendString("@Encoder Fixed: Left=TIM1(PA8-9), Right=TIM4(PB6-7)\r\n");
}

/**
  * 函    数：获取左编码器增量值
  */
int16_t Encoder_GetLeft(void)
{
    int16_t Temp = -TIM_GetCounter(TIM1);
    TIM_SetCounter(TIM1, 0);
    LeftEncoder_Speed = Temp;
	sensor_packet.left_encoder_raw = Temp;
	return Temp;
}

/**
  * 函    数：获取右编码器增量值
  */
int16_t Encoder_GetRight(void)
{
    int16_t Temp = TIM_GetCounter(TIM4);
    TIM_SetCounter(TIM4, 0);
    RightEncoder_Speed = Temp;
	sensor_packet.right_encoder_raw = Temp;
    return Temp;
}

/**
  * 函    数：读取左编码器计数值（不清零）
  */
int16_t Encoder_ReadLeft(void)
{
    return (int16_t)TIM_GetCounter(TIM1);
}

/**
  * 函    数：读取右编码器计数值（不清零）
  */
int16_t Encoder_ReadRight(void)
{
    return (int16_t)TIM_GetCounter(TIM4);
}

/**
  * 函    数：重置编码器计数
  */
void Encoder_Reset(void)
{
    TIM_SetCounter(TIM1, 0);
    TIM_SetCounter(TIM4, 0);
    LeftEncoder_Speed = 0;
    RightEncoder_Speed = 0;
}

