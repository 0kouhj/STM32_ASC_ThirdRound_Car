#include "stdint.h"
#ifndef __SYSTEM_DATA_H
#define __SYSTEM_DATA_H

// 系统运行模式定义
typedef enum {
    SERIAL_BLUETOOTH = 0,
    SERIAL_CH340 = 1
} Serial_Mode;

/*------------------------------------------------------------*/
// 转向状态
typedef enum {
    STEERING_STRAIGHT = 0, // 直行
    STEERING_LEFT,         // 左转
    STEERING_RIGHT,        // 右转
    STEERING_SHARP_LEFT,   // 急左转
    STEERING_SHARP_RIGHT   // 急右转
} Steering_State;
//路口状态
typedef enum {
    // 基础循迹状态
    TRACKING_STRAIGHT      = 0,    // 直行状态
    
    // 细分转向状态
    TRACKING_LEFT_SLIGHT   = 1,    // 轻微左转
    TRACKING_RIGHT_SLIGHT  = 2,    // 轻微右转
    TRACKING_LEFT_MODERATE = 3,    // 中等左转 - 新增
    TRACKING_RIGHT_MODERATE= 4,    // 中等右转 - 新增
    TRACKING_LEFT_SHARP    = 5,    // 急左转
    TRACKING_RIGHT_SHARP   = 6,    // 急右转
    TRACKING_LEFT_EXTREME  = 7,    // 极端左转 - 新增
    TRACKING_RIGHT_EXTREME = 8,    // 极端右转 - 新增
    
    // 特殊元素识别状态
    TRACKING_CROSS_START   = 10,
    TRACKING_CROSS_INSIDE  = 11,
    TRACKING_CROSS_END     = 12,
    
    // 异常和恢复状态
    TRACKING_LOST          = 20,
    TRACKING_RECOVERING    = 21,
    TRACKING_ALL_BLACK     = 22,
    
    // 调试和准备状态
    TRACKING_READY         = 30,
    TRACKING_FINISH        = 31
} TrackingState_t;

typedef enum {
    MODE_MANUAL = 0,       // 手动控制模式
    MODE_AUTO = 1,         // 自动循迹模式
    MODE_BLUETOOTH = 2     // 蓝牙遥控模式
} System_Control_Mode;

// 系统状态数据包
typedef struct {
    // 运动状态
    Steering_State steering_state;    // 转向状态
    uint8_t line_lost;               // 是否丢失路线 (0/1)
    TrackingState_t trackingstate_t;
	// 运行状态
    System_Control_Mode system_control_mode;
    // 电源状态
    uint8_t battery_level;           // 电池电量百分比 (0-100%)
    float battery_voltage;           // 电池电压 (V)
    uint8_t low_battery_warning;     // 低电量警告 (0/1)
    
    // 通信状态
    uint8_t bluetooth_connected;     // 蓝牙连接状态 (0/1)
    
    // 系统状态
    uint8_t system_mode;             // 系统运行模式
    uint8_t emergency_stop;          // 急停标志 (0/1)
} System_Status_Packet;

// 声明全局变量（在头文件中使用extern）
extern System_Status_Packet system_status_packet;
void Data_Update(void);
//紧急停止函数
void Emergency_Stop_Execute(void);
void Emergency_Stop_Recover(void);
uint8_t Is_Emergency_Stop_Active(void);
#endif
