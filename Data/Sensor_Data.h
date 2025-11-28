#include "stdint.h"
#ifndef __SENSOR_DATA_H
#define __SENSOR_DATA_H

// 摇杆数据结构体
typedef struct {
    int16_t x;              // X轴值 (-100 ~ 100)
    int16_t y;              // Y轴值 (-100 ~ 100)
    float sensitivity;      // 灵敏度系数 (0.1 ~ 2.0)
} Joystick_Data;

// 调试数据包
typedef struct {
    // 传感器原始数据
    uint8_t ir_sensor_raw[4];         // 4个红外传感器原始值 [PA7, PA10, PA11, PA12]
    uint16_t left_encoder_raw;        // 左编码器原始计数值
    uint16_t right_encoder_raw;       // 右编码器原始计数值
    uint16_t battery_adc_raw;         // 电池ADC原始值
    
    // 摇杆数据
    Joystick_Data joystick;           // 蓝牙摇杆数据

} Sensor_Packet;
extern Sensor_Packet sensor_packet;
extern Joystick_Data joystick_data;
#endif
