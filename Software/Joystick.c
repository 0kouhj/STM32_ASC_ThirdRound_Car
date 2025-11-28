#include "joystick.h"
#include "motor_data.h"
#include "system_data.h"
#include "motor.h"
#include "serial.h"
#include <stdio.h>

// 静态变量存储摇杆数据
static int joystick_x = 0;
static int joystick_y = 0;
static uint8_t new_data_available = 0;

/**
  * 函    数：处理摇杆数据（主循环调用）
  */
void Joystick_ProcessData(void)
{
    // 只有在蓝牙模式下且有新数据时才处理
    if (system_status_packet.system_control_mode != MODE_BLUETOOTH || !new_data_available) {
        return;
    }
    
    // 验证数据范围
    if (joystick_x < -100) joystick_x = -100;
    if (joystick_x > 100) joystick_x = 100;
    if (joystick_y < -100) joystick_y = -100;
    if (joystick_y > 100) joystick_y = 100;
    
    // 差速转向控制算法
    int left_speed = joystick_y + joystick_x;   // 左电机速度 = 前进分量 + 转向分量
    int right_speed = joystick_y - joystick_x;  // 右电机速度 = 前进分量 - 转向分量
    
    // 限幅处理，确保速度在-100到100之间
    if (left_speed > 100) left_speed = 100;
    if (left_speed < -100) left_speed = -100;
    if (right_speed > 100) right_speed = 100;
    if (right_speed < -100) right_speed = -100;
    
    // 设置电机目标速度
    motor_speed_data.left_target_speed = left_speed;
    motor_speed_data.right_target_speed = right_speed;
    
    // 标记数据已处理
    new_data_available = 0;
}

/**
  * 函    数：设置摇杆数据（供communication.c调用）
  * 说    明：这个函数需要在communication.c中声明为extern
  */
void Joystick_SetData(int x, int y)
{
    joystick_x = x;
    joystick_y = y;
    new_data_available = 1;
}
