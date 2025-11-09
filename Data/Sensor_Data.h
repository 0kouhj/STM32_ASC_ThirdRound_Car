#include "stdint.h"

// 调试数据包
typedef struct {
    // 传感器原始数据
    uint8_t ir_sensor_raw[4];         // 4个红外传感器原始值 [PA7, PA10, PA11, PA12]
    uint16_t left_encoder_raw;        // 左编码器原始计数值
    uint16_t right_encoder_raw;       // 右编码器原始计数值
    uint16_t battery_adc_raw;         // 电池ADC原始值
    

} Sensor_Packet;
extern Sensor_Packet sensor_packet;
