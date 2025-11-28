#include "stm32f10x.h"
#include "Communication.h"
#include "system_data.h"
#include "motor_data.h"
#include "sensor_data.h"
#include "motor.h"
#include "serial.h"
#include "menu.h"
#include "string.h"
#include "stdio.h"
// 串口接收缓冲区
#define RX_BUFFER_SIZE 128
static uint8_t rx_buffer[RX_BUFFER_SIZE];
static volatile uint16_t rx_index = 0;
static volatile uint8_t command_ready = 0;

/**
  * 函    数：串口命令初始化
  */
void Serial_Command_Init(void)
{
    rx_index = 0;
    command_ready = 0;
    memset(rx_buffer, 0, RX_BUFFER_SIZE);
}

/**
  * 函    数：串口接收中断处理
  * 说    明：在USART2_IRQHandler中调用
  */
void Serial_Command_RxHandler(uint8_t data)
{
    static uint8_t rx_state = 0;  // 0:等待起始符, 1:接收普通命令, 2:接收蓝牙数据
    
    if (rx_state == 0) {
        // 等待起始符
        if (data == '[') {
            rx_state = 1;  // 进入普通命令接收状态
            rx_index = 0;
            rx_buffer[rx_index++] = data;
        }
    }
    else if (rx_state == 1) {
        // 接收普通命令（以]结束）
        if (rx_index < RX_BUFFER_SIZE - 1) {
            if (data == ']') {
                if (rx_index > 1) {
                    rx_buffer[rx_index] = '\0';
                    command_ready = 1;
                }
                rx_state = 0;
                rx_index = 0;
            } else {
                rx_buffer[rx_index++] = data;
            }
        } else {
            // 缓冲区溢出
            rx_state = 0;
            rx_index = 0;
        }
    }
}

/**
  * 函    数：检查是否有新命令
  */
uint8_t Serial_Command_Available(void)
{
    return command_ready;
}

/**
  * 函    数：获取当前命令
  */
char* Serial_Command_GetCommand(void)
{
    return (char*)rx_buffer;
}

/**
  * 函    数：清除命令就绪标志
  */
void Serial_Command_ClearFlag(void)
{
    command_ready = 0;
}

/**
  * 函    数：系统控制命令处理
  */
static void Handle_System_Command(const char* command)
{
    char response[64];
    if (strcmp(command, "[SYSTEM STATUS]") == 0) {
        sprintf(response,"Battery:%d\r\nBlueTooth_Connect:%d\r\nMode:",system_status_packet.battery_level,system_status_packet.low_battery_warning);
        Serial_SendString(response);
        switch (system_status_packet.system_control_mode)
        {
            case 0:
                Serial_SendString("MANUAL\r\n");
                break;
            case 1:
                Serial_SendString("AUTO\r\n");
                break;
            case 2:
                Serial_SendString("BLUETOOTH\r\n");
                break;
        }
    }
    else if (strcmp(command, "[STOP]") == 0) {
        Emergency_Stop_Execute();
        Serial_SendString("[OK STOP]\r\n");
    }
    else if (strcmp(command, "[RESET]") == 0) {
        Motor_SetSpeed_Left(0);
        Motor_SetSpeed_Right(0);
        motor_speed_data.left_target_speed = 0;
        motor_speed_data.right_target_speed = 0;
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
        
        Menu_Init();
        
        Serial_SendString("[OK RESET]\r\n");
    }
}

/**
  * 函    数：模式切换命令处理
  */
static void Handle_Mode_Command(const char* command)
{
    if (strcmp(command, "[MODE MANUAL]") == 0) {
        system_status_packet.system_control_mode = MODE_MANUAL;
        Serial_SendString("[OK MODE MANUAL]\r\n");
    }
    else if (strcmp(command, "[MODE AUTO]") == 0) {
        system_status_packet.system_control_mode = MODE_AUTO;
        Serial_SendString("[OK MODE AUTO]\r\n");
    }
    else if (strcmp(command, "[MODE BLUETOOTH]") == 0) {
        system_status_packet.system_control_mode = MODE_BLUETOOTH;
        Serial_SendString("[OK MODE BLUETOOTH]\r\n");
    }
}

/**
  * 函    数：电机速度命令处理
  */
static void Handle_Motor_Command(const char* command)
{
    int left_speed, right_speed;
    if (sscanf(command, "[SL %d]", &left_speed) == 1) {
        motor_speed_data.left_target_speed = left_speed;
    }
    else if (sscanf(command, "[SR %d]", &right_speed) == 1) {
        motor_speed_data.right_target_speed = right_speed;
    }
    else if (sscanf(command, "[S %d]", &left_speed) == 1) {
        motor_speed_data.left_target_speed = left_speed;
        motor_speed_data.right_target_speed = left_speed;
    }
}

/**
  * 函    数：PID参数命令处理
  */
static void Handle_PID_Command(const char* command)
{
    float kp, ki, kd;
    char response[100];
    
    if (strcmp(command, "[PID DEFAULT]") == 0) {
        // 恢复默认PID参数
        pid_parameters.kp_left = 0.08f;
        pid_parameters.ki_left = 0.08f;
        pid_parameters.kd_left = 0.2f;
        pid_parameters.kp_right = 0.08f;
        pid_parameters.ki_right = 0.08f;
        pid_parameters.kd_right = 0.2f;
        pid_parameters.integral_limit = 100.0f;
        pid_parameters.output_limit = 100.0f;
        
        Serial_SendString("[OK PID DEFAULT]\r\n");
    }
    else if (strcmp(command, "[PID SHOW]") == 0) {
        // 显示当前PID参数
        sprintf(response, 
            "[PID PARAMS LEFT %.3f,%.3f,%.3f RIGHT %.3f,%.3f,%.3f LIMITS %.1f,%.1f]\r\n",
            pid_parameters.kp_left, pid_parameters.ki_left, pid_parameters.kd_left,
            pid_parameters.kp_right, pid_parameters.ki_right, pid_parameters.kd_right,
            pid_parameters.integral_limit, pid_parameters.output_limit);
        Serial_SendString(response);
    }
    else if (sscanf(command, "[PID LEFT %f,%f,%f]", &kp, &ki, &kd) == 3) {
        // 设置左电机PID参数
        pid_parameters.kp_left = kp;
        pid_parameters.ki_left = ki;
        pid_parameters.kd_left = kd;
        
        sprintf(response, "[OK PID LEFT %.3f,%.3f,%.3f]\r\n", kp, ki, kd);
        Serial_SendString(response);
    }
    else if (sscanf(command, "[PID RIGHT %f,%f,%f]", &kp, &ki, &kd) == 3) {
        // 设置右电机PID参数
        pid_parameters.kp_right = kp;
        pid_parameters.ki_right = ki;
        pid_parameters.kd_right = kd;
        
        sprintf(response, "[OK PID RIGHT %.3f,%.3f,%.3f]\r\n", kp, ki, kd);
        Serial_SendString(response);
    }
    else if (sscanf(command, "[PID BOTH %f,%f,%f]", &kp, &ki, &kd) == 3) {
        // 同时设置左右电机PID参数
        pid_parameters.kp_left = kp;
        pid_parameters.ki_left = ki;
        pid_parameters.kd_left = kd;
        pid_parameters.kp_right = kp;
        pid_parameters.ki_right = ki;
        pid_parameters.kd_right = kd;
        
        sprintf(response, "[OK PID BOTH %.3f,%.3f,%.3f]\r\n", kp, ki, kd);
        Serial_SendString(response);
    }
    else if (sscanf(command, "[PID LIMITS %f,%f]", &kp, &ki) == 2) {
        // 设置限制参数（假设kp=积分限制, ki=输出限制）
        pid_parameters.integral_limit = kp;
        pid_parameters.output_limit = ki;
        
        sprintf(response, "[OK PID LIMITS %.1f,%.1f]\r\n", kp, ki);
        Serial_SendString(response);
    }
}

/**
  * 函    数：传感器查询命令处理
  */
static void Handle_Sensor_Command(const char* command)
{
    char response[100];
    
    if (strcmp(command, "[SENSOR IR]") == 0) {
        // 读取红外传感器并返回状态
        sprintf(response, "[SENSOR IR PA7=%d PA10=%d PA11=%d PA12=%d]\r\n", 
                sensor_packet.ir_sensor_raw[0], sensor_packet.ir_sensor_raw[1], sensor_packet.ir_sensor_raw[2], sensor_packet.ir_sensor_raw[3]);
        Serial_SendString(response);
    }
    else if (strcmp(command, "[SENSOR BATTERY]") == 0) {
        sprintf(response, "[SENSOR BATTERY VOLTAGE=%.2fV LEVEL=%d]\r\n",
                system_status_packet.battery_voltage, system_status_packet.battery_level);
        Serial_SendString(response);
    }
    else if (strcmp(command, "[SENSOR ALL]") == 0) {
        sprintf(response, "[SENSOR ALL IR PA7=%d PA10=%d PA11=%d PA12=%d BATTERY %.2fV LEVEL=%d]\r\n", 
                sensor_packet.ir_sensor_raw[0], sensor_packet.ir_sensor_raw[1], sensor_packet.ir_sensor_raw[2], sensor_packet.ir_sensor_raw[3],
                system_status_packet.battery_voltage, system_status_packet.battery_level);
        Serial_SendString(response);
    }
}

/**
  * 函    数：状态查询命令处理
  */
static void Handle_Status_Command(const char* command)
{
    char response[100];
    if (strcmp(command, "[STATUS SYSTEM]") == 0) {
        sprintf(response, 
            "[STATUS SYSTEM MODE=%d BATTERY=%d]\r\n"
            "MODE: 0=MANUAL 1=AUTO 2=BLUETOOTH\r\n",
            system_status_packet.system_control_mode, system_status_packet.battery_level);
        Serial_SendString(response);
    }
    else if (strcmp(command, "[STATUS MOTOR]") == 0) {
        sprintf(response,
            "[STATUS MOTOR LEFT ACTUAL=%d TARGET=%d RIGHT ACTUAL=%d TARGET=%d]\r\n",
            motor_speed_data.left_actual_speed, motor_speed_data.left_target_speed,
            motor_speed_data.right_actual_speed, motor_speed_data.right_target_speed);
        Serial_SendString(response);
    }
}

/**
  * 函    数：差速控制计算
  * 说    明：根据摇杆X,Y值计算左右轮速度
  */
static void Calculate_Differential_Speed(int x, int y)
{
    int left_speed, right_speed;
    
    // 差速控制公式
    left_speed = y + x;   // 左轮速度 = 前进速度 + 转向分量
    right_speed = y - x;  // 右轮速度 = 前进速度 - 转向分量
    
    // 限制速度范围 (-90 ~ 90)
    left_speed = left_speed > 90 ? 90 : (left_speed < -90 ? -90 : left_speed);
    right_speed = right_speed > 90 ? 90 : (right_speed < -90 ? -90 : right_speed);
    
    // 设置电机目标速度
    motor_speed_data.left_target_speed = left_speed;
    motor_speed_data.right_target_speed = right_speed;
}

/**
  * 函    数：蓝牙摇杆命令处理
  * 说    明：解析 [joystick,x,y,0,0] 格式数据
  */
static void Handle_Joystick_Command(const char* command)
{
    int x, y, dummy1, dummy2;
    
    // 解析摇杆数据
    if (sscanf(command, "[joystick,%d,%d,%d,%d]", &x, &y, &dummy1, &dummy2) == 4) {
        // 存储到结构体
        sensor_packet.joystick.x = x;
        sensor_packet.joystick.y = y;
        
        // 应用灵敏度
        x = (int)(x * joystick_data.sensitivity);
        y = (int)(y * joystick_data.sensitivity);
        
        // 限制范围
        x = x > 100 ? 100 : (x < -100 ? -100 : x);
        y = y > 100 ? 100 : (y < -100 ? -100 : y);
        
        // 差速控制计算
        Calculate_Differential_Speed(x, y);
    }
}


/**
  * 函    数：摇杆灵敏度设置
  */
static void Handle_Joystick_Sensitivity_Command(const char* command)
{
    float sensitivity;
    
    if (sscanf(command, "[JOYSTICK SENSITIVITY %f]", &sensitivity) == 1) {
        if (sensitivity >= 0.1f && sensitivity <= 2.0f) {
            joystick_data.sensitivity = sensitivity;
            sensor_packet.joystick.sensitivity = sensitivity;
            Serial_SendString("[OK JOYSTICK SENSITIVITY SET]\r\n");
        } else {
            Serial_SendString("[ERROR INVALID SENSITIVITY RANGE 0.1-2.0]\r\n");
        }
    }
}

/**
  * 函    数：帮助命令处理
  */
static void Handle_HELP_Command(const char* command)
{
    char response[200];
    
    if (strcmp(command, "[HELP]") == 0) {
        sprintf(response, 
            "[HELP COMMANDS]\r\n"
            "SYSTEM: [SYSTEM STATUS] [STOP] [RESET]\r\n"
            "MODE: [MODE MANUAL] [MODE AUTO] [MODE BLUETOOTH]\r\n"
            "MOTOR: [SL speed] [SR speed] [S speed]\r\n"
            "PID: [PID DEFAULT] [PID SHOW] [PID LEFT kp,ki,kd]\r\n"
            "      [PID RIGHT kp,ki,kd] [PID BOTH kp,ki,kd] [PID LIMITS i_limit,o_limit]\r\n"
            "SENSOR: [SENSOR IR] [SENSOR BATTERY] [SENSOR ALL]\r\n"
            "STATUS: [STATUS SYSTEM] [STATUS MOTOR]\r\n"
            "JOYSTICK: [joystick,x,y,0,0] [JOYSTICK SENSITIVITY value]\r\n"
            "HELP: [HELP]\r\n"
        );
        Serial_SendString(response);
    }
}

/**
  * 函    数：主命令解析函数
  * 说    明：在主循环中调用，处理接收到的命令
  */
void Serial_Command_Process(void)
{
    if (!command_ready) return;
    
    char* command = Serial_Command_GetCommand();
    
    // 根据命令前缀分发给不同的处理函数
    if (strncmp(command, "[SYSTEM", 7) == 0 || 
        strncmp(command, "[STOP", 5) == 0 || 
        strncmp(command, "[RESET", 6) == 0) {
        Handle_System_Command(command);
    }
    else if (strncmp(command, "[MODE", 5) == 0) {
        Handle_Mode_Command(command);
    }
    else if (strncmp(command, "[S", 2) == 0) {
        Handle_Motor_Command(command);
    }
    else if (strncmp(command, "[PID", 4) == 0) {
        Handle_PID_Command(command);
    }
    else if (strncmp(command, "[SENSOR", 7) == 0) {
        Handle_Sensor_Command(command);
    }
    else if (strncmp(command, "[STATUS", 7) == 0) {
        Handle_Status_Command(command);
    }
    else if (strncmp(command, "[HELP", 5) == 0) {
        Handle_HELP_Command(command);
    }
    else if (strncmp(command, "[joystick", 9) == 0) {
        Handle_Joystick_Command(command);
    }
    else if (strncmp(command, "[JOYSTICK SENSITIVITY", 20) == 0) {
        Handle_Joystick_Sensitivity_Command(command);
    }
    else {
        // 未知命令
        char response[50];
        sprintf(response, "[ERROR UNKNOWN COMMAND: %s]\r\n", command);
        Serial_SendString(response);
    }
    // 清除命令标志
    Serial_Command_ClearFlag();
}
