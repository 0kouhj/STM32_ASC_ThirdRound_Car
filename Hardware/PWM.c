#include "stm32f10x.h"
#include "serial.h"
#include "Motor_Control.h"
#include "Motor_Data.h"
#include "System_Data.h"
#include "Sensor_Data.h"
#include "Encoder.h"
#include "KEY.h"
// 在PWM.c文件开头添加外部声明
extern volatile uint32_t millis_count;


char Motor_Str[64];
/**
  * 函    数：左电机PWM初始化 (PA0, TIM2_CH1)
  */
/**
  * 函    数：左电机PWM初始化 (PA0, TIM2_CH1)
  */
void PWM_LeftMotor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* 开启时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    
    /* 配置GPIO PA0为复用推挽输出 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 配置时基单元 */
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    // 在PWM初始化中修改
	TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;        // 回到1kHz
	TIM_TimeBaseInitStructure.TIM_Prescaler = 36 - 1;       // 72分频
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
    
    /* 配置输出比较通道1 */
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;  // 初始占空比0%
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    
    /* 配置TIM2更新中断 */
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);  // 使能更新中断
    
    /* 配置NVIC */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    TIM_Cmd(TIM2, ENABLE);
}

/**
  * 函    数：右电机PWM初始化 (PA6, TIM3_CH1)
  */
void PWM_RightMotor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    /* 开启时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    
    /* 配置GPIO PA6为复用推挽输出 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 配置时基单元 */
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    // 在PWM初始化中修改
	TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;        // 回到1kHz
	TIM_TimeBaseInitStructure.TIM_Prescaler = 36 - 1;       // 72分频
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
    
    /* 配置输出比较通道1 */
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;  // 初始占空比0%
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
    
    TIM_Cmd(TIM3, ENABLE);
}

/**
  * 函    数：设置左电机PWM占空比
  */
void PWM_SetLeftMotorCompare(uint16_t Compare)
{
    if (Compare > 1000) Compare = 1000;
    TIM_SetCompare1(TIM2, Compare);
}

/**
  * 函    数：设置右电机PWM占空比
  */
void PWM_SetRightMotorCompare(uint16_t Compare)
{
    if (Compare > 1000) Compare = 1000;
    TIM_SetCompare1(TIM3, Compare);
}
void Data_Update(void)
{
    // 更新电池状态（修正判断逻辑）
    if (system_status_packet.battery_voltage < 10.0f)  // 修正：通常低电量在7V左右
    {
        system_status_packet.low_battery_warning = 1;
        system_status_packet.battery_level = (uint8_t)((system_status_packet.battery_voltage / 12.6f) * 100);
    }
    else
    {
        system_status_packet.low_battery_warning = 0;
        system_status_packet.battery_level = (uint8_t)((system_status_packet.battery_voltage / 12.6f) * 100);
    }
	//Serial_SendString("Data Update Successfully");
}
// 在pwm.c中的TIM2中断处理函数


// 修改TIM2中断处理函数
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        Key_Tick();
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}
