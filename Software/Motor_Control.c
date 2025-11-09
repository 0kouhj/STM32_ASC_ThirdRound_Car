#include "stm32f10x.h"
#include "motor_control.h"
#include "motor.h"
#include "encoder.h"
#include "Serial.h"
#include "motor_data.h"

// 定义全局变量
Motor_Speed_Data motor_speed_data = {
    .left_target_speed = 0,
    .left_actual_speed = 0,
    .right_target_speed = 0,
    .right_actual_speed = 0,
    .left_error = 0,
    .left_previous_error = 0,
    .left_integral = 0,
    .right_error = 0,
    .right_previous_error = 0,
    .right_integral = 0,
    .left_pid_output = 0,
    .right_pid_output = 0
};

// PID参数
PID_Parameters pid_parameters = {
    .kp_left = 0.08f,
    .ki_left = 0.08f,
    .kd_left = 0.2f,
    .kp_right = 0.08f,
    .ki_right = 0.08f,
    .kd_right = 0.2f,
    .integral_limit = 100.0f,
    .output_limit = 100.0f
};

void MotorControl_Init(void)
{
    // 初始化电机速度数据
    motor_speed_data.left_target_speed = 0;
    motor_speed_data.right_target_speed = 0;
    motor_speed_data.left_actual_speed = 0;
    motor_speed_data.right_actual_speed = 0;
    
    // 初始化PID相关变量
    motor_speed_data.left_error = 0;
    motor_speed_data.left_previous_error = 0;
    motor_speed_data.left_integral = 0;
    motor_speed_data.left_pid_output = 0;
    
    motor_speed_data.right_error = 0;
    motor_speed_data.right_previous_error = 0;
    motor_speed_data.right_integral = 0;
    motor_speed_data.right_pid_output = 0;
}

void MotorControl_Update(void)
{
    // 获取编码器速度
    motor_speed_data.left_actual_speed = (int16_t)Encoder_GetLeft()*1.4;
    motor_speed_data.right_actual_speed = (int16_t)Encoder_GetRight()*1.4;

    // 如果目标速度为0，停止电机并重置积分
    if(motor_speed_data.left_target_speed == 0 && motor_speed_data.right_target_speed == 0) {
        Motor_SetSpeed_Left(0);
        Motor_SetSpeed_Right(0);
        motor_speed_data.left_integral = 0;
        motor_speed_data.right_integral = 0;
        motor_speed_data.left_previous_error = 0;
        motor_speed_data.right_previous_error = 0;
        return;
    }

    // 左电机PID计算 - 按照您提供的思路
    motor_speed_data.left_previous_error = motor_speed_data.left_error;
    motor_speed_data.left_error = motor_speed_data.left_target_speed - motor_speed_data.left_actual_speed;
    
    motor_speed_data.left_integral += motor_speed_data.left_error;
    
    // 积分限幅
    if(motor_speed_data.left_integral > pid_parameters.integral_limit) 
        motor_speed_data.left_integral = pid_parameters.integral_limit;
    if(motor_speed_data.left_integral < -pid_parameters.integral_limit) 
        motor_speed_data.left_integral = -pid_parameters.integral_limit;
    
    // PID计算
    motor_speed_data.left_pid_output = pid_parameters.kp_left * motor_speed_data.left_error + 
                                      pid_parameters.ki_left * motor_speed_data.left_integral + 
                                      pid_parameters.kd_left * (motor_speed_data.left_error - motor_speed_data.left_previous_error);
    
    // 输出限幅
    if (motor_speed_data.left_pid_output > pid_parameters.output_limit) 
        motor_speed_data.left_pid_output = pid_parameters.output_limit;
    if (motor_speed_data.left_pid_output < -pid_parameters.output_limit) 
        motor_speed_data.left_pid_output = -pid_parameters.output_limit;

    // 右电机PID计算 - 按照您提供的思路
    motor_speed_data.right_previous_error = motor_speed_data.right_error;
    motor_speed_data.right_error = motor_speed_data.right_target_speed - motor_speed_data.right_actual_speed;
    
    motor_speed_data.right_integral += motor_speed_data.right_error;
    
    // 积分限幅
    if(motor_speed_data.right_integral > pid_parameters.integral_limit) 
        motor_speed_data.right_integral = pid_parameters.integral_limit;
    if(motor_speed_data.right_integral < -pid_parameters.integral_limit) 
        motor_speed_data.right_integral = -pid_parameters.integral_limit;
    
    // PID计算
    motor_speed_data.right_pid_output = pid_parameters.kp_right * motor_speed_data.right_error + 
                                       pid_parameters.ki_right * motor_speed_data.right_integral + 
                                       pid_parameters.kd_right * (motor_speed_data.right_error - motor_speed_data.right_previous_error);
    
    // 输出限幅
    if (motor_speed_data.right_pid_output > pid_parameters.output_limit) 
        motor_speed_data.right_pid_output = pid_parameters.output_limit;
    if (motor_speed_data.right_pid_output < -pid_parameters.output_limit) 
        motor_speed_data.right_pid_output = -pid_parameters.output_limit;
    
    // 设置电机速度
    Motor_SetSpeed_Left((int8_t)motor_speed_data.left_pid_output);
    Motor_SetSpeed_Right((int8_t)motor_speed_data.right_pid_output);
}

void MotorControl_ResetIntegral(void)
{
    motor_speed_data.left_integral = 0;
    motor_speed_data.right_integral = 0;
    motor_speed_data.left_previous_error = 0;
    motor_speed_data.right_previous_error = 0;
}
