#ifndef __MOTOR_DATA_H
#define __MOTOR_DATA_H

#include <stdint.h>

// 电机速度数据
typedef struct {
    int16_t left_target_speed;       // 左电机目标速度 (-100 to +100)
    int16_t left_actual_speed;       // 左电机实际速度 (-100 to +100)
    int16_t right_target_speed;      // 右电机目标速度 (-100 to +100)
    int16_t right_actual_speed;      // 右电机实际速度 (-100 to +100)
    
    // 新增：PID误差相关变量
    float left_error;                // 左电机当前误差
    float left_previous_error;       // 左电机上一次误差
    float left_integral;             // 左电机积分项
    
    float right_error;               // 右电机当前误差
    float right_previous_error;      // 右电机上一次误差
    float right_integral;            // 右电机积分项
    
    // PID输出
    float left_pid_output;           // 左电机PID输出
    float right_pid_output;          // 右电机PID输出
} Motor_Speed_Data;

// PID参数（对应@PID命令）
typedef struct {
    float kp_left, ki_left, kd_left;    // 左电机PID参数
    float kp_right, ki_right, kd_right; // 右电机PID参数
    
    float integral_limit;            // 积分项限制
    float output_limit;              // 输出限制
} PID_Parameters;

extern Motor_Speed_Data motor_speed_data;
extern PID_Parameters pid_parameters;
#endif
