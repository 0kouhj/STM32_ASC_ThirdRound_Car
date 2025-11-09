#include "stm32f10x.h"
#include "Communication.h"
#include "system_data.h"
#include "motor_data.h"
#include "sensor_data.h"
#include "motor.h"
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
    // 检查是否为命令起始符 '@'
    if (data == '@' && rx_index == 0) {
        rx_buffer[rx_index++] = data;
        return;
    }
    
    // 如果已经开始接收命令，继续存储
    if (rx_index > 0 && rx_index < RX_BUFFER_SIZE - 1) {
        // 命令结束符检测（回车或换行）
        if (data == '\r' || data == '\n') {
            if (rx_index > 1) { // 确保不是空命令 "@"
                rx_buffer[rx_index] = '\0'; // 字符串结束符
                command_ready = 1;
            }
            rx_index = 0;
        } else {
            rx_buffer[rx_index++] = data;
        }
    } else {
        // 缓冲区溢出，重置
        rx_index = 0;
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
    if (strcmp(command, "@SYSTEM STATUS") == 0) {
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
    else if (strcmp(command, "@STOP") == 0) {
		motor_speed_data.left_target_speed=0;
		motor_speed_data.right_target_speed=0;
		Motor_SetSpeed_Left(0);
		Motor_SetSpeed_Right(0);
        Serial_SendString("@OK STOP\r\n");
    }
    else if (strcmp(command, "@RESET") == 0) {
		Motor_SetSpeed_Left(0);
		Motor_SetSpeed_Right(0);
        Serial_SendString("@OK RESET\r\n");
        // 这里可以添加系统复位逻辑
    }
}

/**
  * 函    数：模式切换命令处理
  */
static void Handle_Mode_Command(const char* command)
{
    if (strcmp(command, "@MODE MANUAL") == 0) {
		system_status_packet.system_control_mode = MODE_MANUAL;
        Serial_SendString("@OK MODE MANUAL\r\n");
    }
    else if (strcmp(command, "@MODE AUTO") == 0) {
		system_status_packet.system_control_mode = MODE_AUTO;
        Serial_SendString("@OK MODE AUTO\r\n");
    }
    else if (strcmp(command, "@MODE BLUETOOTH") == 0) {
		system_status_packet.system_control_mode = MODE_BLUETOOTH;
        Serial_SendString("@OK MODE BLUETOOTH\r\n");
    }
}

/**
  * 函    数：电机速度命令处理
  */
static void Handle_Motor_Command(const char* command)
{
    int left_speed, right_speed;
    char response[50];
    if (sscanf(command, "@SL %d", &left_speed) == 1) {
		motor_speed_data.left_target_speed = left_speed;
        Serial_SendString(response);
    }
    else if (sscanf(command, "@SR %d", &right_speed) == 1) {
		motor_speed_data.right_target_speed = right_speed;
        Serial_SendString(response);
    }
    else if (sscanf(command, "@S %d", &left_speed) == 1) {
		motor_speed_data.left_target_speed = left_speed;
		motor_speed_data.right_target_speed = left_speed;
        Serial_SendString(response);
    }
    else if (sscanf(command, "@SLR %d,%d", &left_speed, &right_speed) == 2) {

        Serial_SendString(response);
    }
}

/**
  * 函    数：运动控制命令处理
  */
static void Handle_Move_Command(const char* command)
{
    int speed;
    char response[50];
    
    if (sscanf(command, "@MOVE F %d", &speed) == 1) {

        sprintf(response, "@OK MOVE F %d\r\n", speed);
        Serial_SendString(response);
    }
    else if (sscanf(command, "@MOVE B %d", &speed) == 1) {

        sprintf(response, "@OK MOVE B %d\r\n", speed);
        Serial_SendString(response);
    }
    else if (sscanf(command, "@MOVE L %d", &speed) == 1) {

        sprintf(response, "@OK MOVE L %d\r\n", speed);
        Serial_SendString(response);
    }
    else if (sscanf(command, "@MOVE R %d", &speed) == 1) {

        sprintf(response, "@OK MOVE R %d\r\n", speed);
        Serial_SendString(response);
    }
    else if (sscanf(command, "@SPIN L %d", &speed) == 1) {

        sprintf(response, "@OK SPIN L %d\r\n", speed);
        Serial_SendString(response);
    }
    else if (sscanf(command, "@SPIN R %d", &speed) == 1) {

        sprintf(response, "@OK SPIN R %d\r\n", speed);
        Serial_SendString(response);
    }
}

/**
  * 函    数：PID参数命令处理
  */
static void Handle_PID_Command(const char* command)
{
    float kp, ki, kd;
    char response[50];
    
    if (strcmp(command, "@PID DEFAULT") == 0) {
        // 恢复默认PID参数

    }
    else if (strcmp(command, "@PID SHOW") == 0) {

    }
    else if (sscanf(command, "@PID LEFT %f,%f,%f", &kp, &ki, &kd) == 3) {
		pid_parameters.kd_left =kd;pid_parameters.kp_left=kp;pid_parameters.ki_left = ki;
        sprintf(response, "@OK PID LEFT %.2f,%.2f,%.2f\r\n", kp, ki, kd);
        Serial_SendString(response);
    }
    else if (sscanf(command, "@PID RIGHT %f,%f,%f", &kp, &ki, &kd) == 3) {

        sprintf(response, "@OK PID RIGHT %.2f,%.2f,%.2f\r\n", kp, ki, kd);
        Serial_SendString(response);
    }
    else if (sscanf(command, "@PID BOTH %f,%f,%f", &kp, &ki, &kd) == 3) {

        sprintf(response, "@OK PID BOTH %.2f,%.2f,%.2f\r\n", kp, ki, kd);
        Serial_SendString(response);
    }
}

/**
  * 函    数：传感器查询命令处理
  */
static void Handle_Sensor_Command(const char* command)
{
    char response[100];
    
    if (strcmp(command, "@SENSOR IR") == 0) {
        // 读取红外传感器并返回状态
        sprintf(response, "@SENSOR IR PA7=%d PA10=%d PA11=%d PA12=%d\r\n", 
                0, 0, 0, 0); // 这里需要替换为实际传感器读取
        Serial_SendString(response);
    }
    else if (strcmp(command, "@SENSOR ENCODER") == 0) {


        Serial_SendString(response);
    }
    else if (strcmp(command, "@SENSOR BATTERY") == 0) {
        sprintf(response, "@SENSOR BATTERY VOLTAGE=12.3V LEVEL=85\r\n"); // 示例数据
        Serial_SendString(response);
    }
    else if (strcmp(command, "@SENSOR ALL") == 0) {
        Serial_SendString(response);
    }
}

/**
  * 函    数：状态查询命令处理
  */
static void Handle_Status_Command(const char* command)
{
    char response[100];
    if (strcmp(command, "@STATUS SYSTEM") == 0) {


        Serial_SendString(response);
    }
    else if (strcmp(command, "@STATUS MOTOR") == 0) {


        Serial_SendString(response);
    }
    else if (strcmp(command, "@STATUS DEBUG") == 0) {


        Serial_SendString(response);
    }
    else if (strcmp(command, "@STATUS ALL") == 0) {


        Serial_SendString(response);
    }
}

/**
  * 函    数：调试命令处理
  */
static void Handle_Debug_Command(const char* command)
{
    if (strcmp(command, "@DEBUG ON") == 0) {

        Serial_SendString("@OK DEBUG ON\r\n");
    }
    else if (strcmp(command, "@DEBUG OFF") == 0) {

        Serial_SendString("@OK DEBUG OFF\r\n");
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
    if (strncmp(command, "@SYSTEM", 6) == 0 || 
        strncmp(command, "@STOP", 5) == 0 || 
        strncmp(command, "@RESET", 6) == 0) {
        Handle_System_Command(command);
    }
    else if (strncmp(command, "@MODE", 5) == 0) {
        Handle_Mode_Command(command);
    }
    else if (strncmp(command, "@S", 2) == 0) {
        Handle_Motor_Command(command);
    }
    else if (strncmp(command, "@MOVE", 5) == 0 || 
             strncmp(command, "@SPIN", 5) == 0) {
        Handle_Move_Command(command);
    }
    else if (strncmp(command, "@PID", 4) == 0) {
        Handle_PID_Command(command);
    }
    else if (strncmp(command, "@SENSOR", 7) == 0) {
        Handle_Sensor_Command(command);
    }
    else if (strncmp(command, "@STATUS", 7) == 0) {
        Handle_Status_Command(command);
    }
    else if (strncmp(command, "@DEBUG", 6) == 0) {
        Handle_Debug_Command(command);
    }
    else {
        // 未知命令
        char response[50];
        sprintf(response, "@ERROR UNKNOWN COMMAND: %s\r\n", command);
        Serial_SendString(response);
    }
	//Serial_SendString("Command Received");
    // 清除命令标志
    Serial_Command_ClearFlag();
}
