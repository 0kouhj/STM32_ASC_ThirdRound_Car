#include "stm32f10x.h"
#include "Motor.h"
#include "Pwm.h"
#include "Encoder.h"
#include "Serial.h"

// 电机速度变量
volatile int32_t LeftMotor_Speed = 0;
volatile int32_t RightMotor_Speed = 0;

/**
  * 函    数：直流电机初始化
  */
void Motor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* 开启GPIOB时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    /* 配置电机方向控制引脚 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
    // 左电机方向控制: PB12(AIN1), PB13(AIN2) - 对应TB6612
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 右电机方向控制: PB14(BIN1), PB15(BIN2) - 对应TB6612
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 初始化方向为停止状态
    GPIO_ResetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
    
    /* 初始化PWM和编码器 */
    PWM_LeftMotor_Init();
    PWM_RightMotor_Init();
    // Encoder_Init();  // 暂时注释掉编码器
    
    Serial_SendString("@Motor Initialized: AIN1=PB12, AIN2=PB13, BIN1=PB14, BIN2=PB15\r\n");
}

/**
  * 函    数：左电机设置速度（增强调试版本）
  * 参    数：Speed - 速度值，-100到100，负值为反转
  */
void Motor_SetSpeed_Left(int8_t Speed)
{
    if (Speed > 90) Speed = 90;
    if (Speed < -90) Speed = -90;
    if (Speed > 0)
    {
        // 正转：AIN1=1, AIN2=0
        GPIO_SetBits(GPIOB, GPIO_Pin_12);   // AIN1 = 1
        GPIO_ResetBits(GPIOB, GPIO_Pin_13); // AIN2 = 0
        PWM_SetLeftMotorCompare(Speed * 10); // 0-1000范围
    }
    else if (Speed < 0)
    {
        // 反转：AIN1=0, AIN2=1
        GPIO_ResetBits(GPIOB, GPIO_Pin_12); // AIN1 = 0
        GPIO_SetBits(GPIOB, GPIO_Pin_13);   // AIN2 = 1
        PWM_SetLeftMotorCompare(-Speed * 10);
    }
    else
    {
        // 停止：AIN1=0, AIN2=0 (刹车)
        GPIO_ResetBits(GPIOB, GPIO_Pin_12); // AIN1 = 0
        GPIO_ResetBits(GPIOB, GPIO_Pin_13); // AIN2 = 0
        PWM_SetLeftMotorCompare(0);
    }
}

/**
  * 函    数：右电机设置速度（增强调试版本）
  * 参    数：Speed - 速度值，-100到100，负值为反转
  */
void Motor_SetSpeed_Right(int8_t Speed)
{
    if (Speed > 90) Speed = 90;
    if (Speed < -90) Speed = -90;

    if (Speed > 0)
    {
        // 正转：BIN1=1, BIN2=0
        GPIO_SetBits(GPIOB, GPIO_Pin_14);   // BIN1 = 1
        GPIO_ResetBits(GPIOB, GPIO_Pin_15); // BIN2 = 0
        PWM_SetRightMotorCompare(Speed * 10);
    }
    else if (Speed < 0)
    {
        // 反转：BIN1=0, BIN2=1
        GPIO_ResetBits(GPIOB, GPIO_Pin_14); // BIN1 = 0
        GPIO_SetBits(GPIOB, GPIO_Pin_15);   // BIN2 = 1
        PWM_SetRightMotorCompare(-Speed * 10);
    }
    else
    {
        // 停止：BIN1=0, BIN2=0 (刹车)
        GPIO_ResetBits(GPIOB, GPIO_Pin_14); // BIN1 = 0
        GPIO_ResetBits(GPIOB, GPIO_Pin_15); // BIN2 = 0
        PWM_SetRightMotorCompare(0);
    }

}

/**
  * 函    数：获取左电机速度
  */
int32_t Motor_GetLeftSpeed(void)
{
    return LeftMotor_Speed;
}

/**
  * 函    数：获取右电机速度
  */
int32_t Motor_GetRightSpeed(void)
{
    return RightMotor_Speed;
}
