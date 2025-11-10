#ifndef __INFRARED_SENSOR_H
#define __INFRARED_SENSOR_H

#include "stm32f10x.h"

// 红外传感器数量
#define INFRARED_SENSOR_COUNT 4

// 传感器引脚定义
typedef enum {
    INFRARED_SENSOR_1 = 0,  // PA7
    INFRARED_SENSOR_2,      // PA10
    INFRARED_SENSOR_3,      // PA11
    INFRARED_SENSOR_4       // PA12
} InfraredSensor_t;

// 传感器状态
typedef enum {
    INFRARED_WHITE = 0,     // 检测到白色（反射强）
    INFRARED_BLACK = 1      // 检测到黑色（反射弱）
} InfraredState_t;

// 函数声明
void Infrared_Init(void);
void Infrared_Update_SensorsState(void);

#endif
