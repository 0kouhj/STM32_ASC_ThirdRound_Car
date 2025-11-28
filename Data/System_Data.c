#include "stm32f10x.h"
#include "Motor_Data.h"
#include "System_Data.h"
#include "Sensor_Data.h"
#include "Encoder.h"
#include "Serial.h"
#include "Infrared_Sensor.h"
#include "Motor.h"
#include "OLED.h"
#include "LED.h"
#include "PWM.h"
#include "BlueTooth.h"
extern uint8_t need_refresh;
// 定义全局变量
System_Status_Packet system_status_packet = {
    .steering_state = STEERING_STRAIGHT,
    .line_lost = 0,
    .system_control_mode = MODE_MANUAL,
    .battery_level = 100,
    .battery_voltage = 12.6f,
    .low_battery_warning = 0,
    .bluetooth_connected = 0,
    .system_mode = 0,
    .emergency_stop = 0
};

void Data_Update(void)
{
	if (Bluetooth_GetHardwareState())
	{
		system_status_packet.bluetooth_connected = 1;
	}
	else
	{
		system_status_packet.bluetooth_connected = 0;
	}
	if (system_status_packet.emergency_stop)
	{
		Emergency_Stop_Execute();
	}
	Infrared_Update_SensorsState();
    // 更新电池状态
    if (system_status_packet.battery_voltage < 10.0f)
    {
        system_status_packet.low_battery_warning = 1;
        system_status_packet.battery_level = (uint8_t)((system_status_packet.battery_voltage / 12.6f) * 100);
    }
    else
    {
        system_status_packet.low_battery_warning = 0;
        system_status_packet.battery_level = (uint8_t)((system_status_packet.battery_voltage / 12.6f) * 100);
    }
}

/**
  * @brief  紧急停止函数 - 停止所有电机并进入省电模式
  * @param  None
  * @retval None
  */
void Emergency_Stop_Execute(void)
{
    // 1. 设置紧急停止标志
    system_status_packet.emergency_stop = 1;
    
    // 2. 立即停止所有电机
    motor_speed_data.left_target_speed = 0;
	motor_speed_data.right_target_speed = 0;

    // 3. 关闭PWM输出
    PWM_SetLeftMotorCompare(0);
    PWM_SetRightMotorCompare(0);
    
    // 4. 禁用电机方向控制引脚
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 左电机方向控制引脚 (PB12, PB13) 设为模拟输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 右电机方向控制引脚 (PB14, PB15) 设为模拟输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 5. 关闭编码器定时器
    TIM_Cmd(TIM1, DISABLE);  // 左编码器
    TIM_Cmd(TIM4, DISABLE);  // 右编码器
    
    // 6. 关闭蓝牙模块
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_8);  // 蓝牙EN置低
    
    // 7. 关闭ADC（电池检测）
    ADC_Cmd(ADC1, DISABLE);
    
    // 8. 配置其他GPIO为模拟输入以省电（除OLED和必要引脚外）
    
    // GPIOA - 除PA13(SWDIO), PA14(SWCLK)外都设为模拟输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | 
                                 GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 |
                                 GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // GPIOB - 除PB10(SCL), PB11(SDA), PB8(蓝牙EN)外都设为模拟输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_6 | GPIO_Pin_7 |
                                 GPIO_Pin_9 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 9. 关闭不必要的外设时钟以省电
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
    // 注意：不要关闭GPIO和AFIO时钟，否则无法操作GPIO
    
    // 10. 点亮LED表示紧急停止状态
    LED_On();
    
    // 11. 在OLED上显示紧急停止信息
	if (!system_status_packet.low_battery_warning)
	{
		OLED_Clear();
		OLED_ShowString(0, 8, "EMERGENCY STOP", OLED_8X16);
		OLED_ShowString(0, 32, "System Halted!", OLED_6X8);
		OLED_ShowString(0, 48, "Press RESET to recover", OLED_6X8);
		OLED_Update();
	}
    
    // 12. 发送紧急停止消息（如果串口还在工作）
    Serial_SendString("[EMERGENCY STOP] System Halted! Press RESET to recover.\r\n");
}

/**
  * @brief  检查紧急停止是否激活
  * @param  None
  * @retval 1:激活, 0:未激活
  */
uint8_t Is_Emergency_Stop_Active(void)
{
    return system_status_packet.emergency_stop;
}

/**
  * @brief  恢复函数（通过重置键复位后自动执行）
  * @param  None
  * @retval None
  */
void Emergency_Stop_Recover(void)
{
    // 这个函数实际上在系统复位后由main()中的初始化代码自动执行
    // 这里只是清除标志
    system_status_packet.emergency_stop = 0;
}

