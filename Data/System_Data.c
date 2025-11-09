#include "stm32f10x.h"
#include "Motor_Data.h"
#include "System_Data.h"
#include "Sensor_Data.h"
#include "Encoder.h"
#include "Serial.h"
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


