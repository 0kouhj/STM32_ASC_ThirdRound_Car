#include "stm32f10x.h"                  
#include "Motor.h"
#include "Serial.h"
#include "Motor_Control.h"
#include "KEY.h"
#include "Delay.h"
#include "Communication.h"
#include "motor_data.h"
#include "Encoder.h"  // 添加Encoder.h头文件
#include "OLED.h"

// 编译时通信模式选择
// #define USE_BLUETOOTH_MODE

#ifdef USE_BLUETOOTH_MODE
    #define COMMUNICATION_MODE_BLUETOOTH
    #define COMMUNICATION_MODE_STRING "BLUETOOTH"
#else
    #define COMMUNICATION_MODE_CH340  
    #define COMMUNICATION_MODE_STRING "CH340"
#endif


// 声明全局毫秒计数器
extern volatile uint32_t millis_count;

static uint16_t Count_Serial = 0;
int main(void)
{
    SystemInit();
    // 初始化所有模块
    Serial_Init();
    Motor_Init();
    Encoder_Init();
    MotorControl_Init();
    KEY_Init();
    Serial_Command_Init();
	OLED_Init();
	OLED_ShowString(0,0,"Hello",OLED_8X16);
	OLED_Update();
    Serial_SendString("@=== Smart Car System Started ===\r\n");
    while(1)
    {
        Serial_Command_Process();
		MotorControl_Update();
		Count_Serial++;

		Serial_Printf("%d,%d,%d,%d\r\n",
                motor_speed_data.left_target_speed,
                motor_speed_data.right_target_speed,
                motor_speed_data.left_actual_speed,
                motor_speed_data.right_actual_speed);
    }
}
