#include "stm32f10x.h"
#include "motor_control.h"
#include "motor.h"
#include "encoder.h"
#include "Serial.h"
#include "motor_data.h"  // 添加这行

// 定义全局变量
Motor_Speed_Data motor_speed_data = {
    .left_target_speed = 0,
    .left_actual_speed = 0,
    .right_target_speed = 0,
    .right_actual_speed = 0
};

// 如果需要PID参数，也定义它
PID_Parameters pid_parameters = {
    .kp_left = 0.8f,
    .ki_left = 0.1f,
    .kd_left = 0.05f,
    .kp_right = 0.8f,
    .ki_right = 0.1f,
    .kd_right = 0.05f
};
