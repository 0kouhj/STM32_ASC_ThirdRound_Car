#include "stm32f10x.h"                  
#include "Delay.h"
#include <string.h>

#include "Timer.h"
#include "Serial.h"
#include "Motor.h"
#include "Encoder.h"
#include "LED.h"
#include "Infrared_Sensor.h"

#include "KEY.h"

#include "Motor_Control.h"
#include "Communication.h"

#include "OLED.h"
#include "Menu.h"


#include "System_Data.h"

// 编译时通信模式选择
#define USE_BLUETOOTH_MODE

#ifdef USE_BLUETOOTH_MODE
    #define COMMUNICATION_MODE_BLUETOOTH
    #define COMMUNICATION_MODE_STRING "BLUETOOTH"
#else
    #define COMMUNICATION_MODE_CH340  
    #define COMMUNICATION_MODE_STRING "CH340"
#endif

// 定义各种计时器
Timer_t display_refresh_timer;
Timer_t data_report_timer; 
Timer_t motor_update_timer;
Timer_t system_status_timer;
Timer_t sensor_update_timer;
Timer_t led_blink_timer;

int main(void)
{
    // 初始化代码
    SystemInit();
    Timer_Init();
    // 初始化所有模块
    Serial_Init();
    Motor_Init();
    Encoder_Init();
	KEY_Init();
	LED_Init();
	Infrared_Init();
	
	OLED_Init();
	
    MotorControl_Init();
    Serial_Command_Init();

	Menu_Init();
    //发送启动信息
	Serial_SendString("'---------智能小车车,启动!!!-----------'\r\n");
	
    // 启动所有计时器
    Timer_Start(&display_refresh_timer);
    Timer_Start(&data_report_timer);
    Timer_Start(&motor_update_timer); 
    Timer_Start(&system_status_timer);
    Timer_Start(&sensor_update_timer);
	Timer_Start(&led_blink_timer);
	
    // 显示启动信息
	OLED_ShowString(0,0, "---------------------", OLED_6X8);
	OLED_ShowString(0,24, "---------------------", OLED_6X8);
    OLED_ShowString(0,6, "->SYSTEM START<-", OLED_8X16);
	OLED_ShowImage(0,32,128,32,BiaoQingBao);
    OLED_ShowString(24,24, COMMUNICATION_MODE_STRING,OLED_6X8);
    OLED_Update();
	
	Delay_s(2);
	
    while(1)
    {
        // 高频任务：每循环都执行
        Serial_Command_Process();
        
        // 中频任务：20ms传感器更新
        if (Timer_IsTimeout(&sensor_update_timer, 20)) {
            Data_Update();
            Timer_Start(&sensor_update_timer);
        }
        
        // 中频任务：10ms
        if (Timer_IsTimeout(&motor_update_timer, 10)) {
			//按键检测
			Key_Tick();
            if (system_status_packet.low_battery_warning == 0 && system_status_packet.emergency_stop == 0)
			{
				// Track_Following_Algorithm();
				
				// 电机控制
				MotorControl_Update();
				Timer_Start(&motor_update_timer);
			}
        }
        
        // 显示任务：100ms刷新
        if (Timer_IsTimeout(&display_refresh_timer, 50)) {
			Menu_Process();
            Timer_Start(&display_refresh_timer);
        }
        
        // 系统检查任务：500ms
        if (Timer_IsTimeout(&system_status_timer, 500)) {

            Timer_Start(&system_status_timer);
        }
		//系统1s闪烁
		if (Timer_IsTimeout(&led_blink_timer, 1000)) {
			if (system_status_packet.low_battery_warning == 0 && system_status_packet.emergency_stop == 0)
			{
				LED_Toggle();
			}
			else	LED_On();
            Timer_Start(&led_blink_timer);
        }
    }
}
