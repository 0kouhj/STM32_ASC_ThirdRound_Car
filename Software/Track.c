#include "Track.h"
#include "Track.h"
#include "motor_data.h"
#include "motor_control.h"
#include "Timer.h"
#include "System_Data.h"
#include "Sensor_Data.h"

// 基础转弯程度参数（简化版本）
#define BASE_SLIGHT_TURN_DIFF 25      // 基础轻微转向差速
#define BASE_MODERATE_TURN_DIFF 40    // 基础中等转向差速  
#define BASE_SHARP_TURN_DIFF 50       // 基础急转差速

// 状态机内部变量
static TrackingState_t current_state = TRACKING_READY;
static uint32_t state_start_time = 0;
static uint32_t cross_start_time = 0;
static uint8_t last_sensor_state = 0;
static uint8_t recovery_direction = 0;

// 控制参数定义
#define STATE_UPDATE_INTERVAL 20
#define CROSS_BASE_TIMEOUT 400
#define RECOVERY_TIMEOUT 10000
#define RECOVERY_SPEED 40

// 速度配置结构体
typedef struct {
    int16_t straight_speed;
    int16_t slight_turn_speed;
    int16_t moderate_turn_speed;
    int16_t sharp_turn_speed;
    int16_t cross_speed;
} SpeedProfile;

// 速度配置实例
static SpeedProfile speed_profile = {
    .straight_speed =50,
    .slight_turn_speed = 35,
    .moderate_turn_speed = 25,
    .sharp_turn_speed = 15,
    .cross_speed = 45
};

void TrackingStateMachine_Init(void)
{
    current_state = TRACKING_READY;
    state_start_time = Timer_GetSystemMillis();
    last_sensor_state = 0;
}

/**
 * @brief 获取前三个传感器的状态
 * 传感器布局：[L, C1, R] - 只使用这三个传感器
 */
static uint8_t GetSensorState(void)
{
    uint8_t state = 0;
    
    // 只使用前三个传感器
    if (sensor_packet.ir_sensor_raw[1] == 0) { // L传感器
        state |= (1 << 2);
    }
    
    if (sensor_packet.ir_sensor_raw[0] == 0) { // C1传感器
        state |= (1 << 1);
    }
    
    if (sensor_packet.ir_sensor_raw[2] == 0) { // R传感器
        state |= (1 << 0);
    }
    
    return state;
}

/**
 * @brief 判断是否为转向状态
 */
static uint8_t IsTurningState(TrackingState_t state)
{
    return (state == TRACKING_LEFT_SLIGHT || state == TRACKING_RIGHT_SLIGHT ||
            state == TRACKING_LEFT_MODERATE || state == TRACKING_RIGHT_MODERATE ||
            state == TRACKING_LEFT_SHARP || state == TRACKING_RIGHT_SHARP);
}

static void ChangeState(TrackingState_t new_state)
{
    if (current_state != new_state) {
        current_state = new_state;
        state_start_time = Timer_GetSystemMillis();
        system_status_packet.trackingstate_t = new_state;
    }
}

static void SetMotorSpeed(int16_t left_speed, int16_t right_speed)
{
    motor_speed_data.left_target_speed = left_speed;
    motor_speed_data.right_target_speed = right_speed;
}

static uint8_t IsAutoTrackingMode(void)
{
    return (system_status_packet.system_control_mode == MODE_AUTO && 
            system_status_packet.emergency_stop == 0);
}

/**
 * @brief 处理直行状态 - 使用三个传感器的逻辑
 */
static void HandleStraightState(uint8_t sensor_state)
{
    switch(sensor_state) {
        case 0x02: // 只有C1: 010 - 完美直行
            SetMotorSpeed(speed_profile.straight_speed, speed_profile.straight_speed);
            break;
            
        case 0x06: // L和C1: 110 - 轻微左偏
            ChangeState(TRACKING_LEFT_SLIGHT);
            break;
            
        case 0x03: // C1和R: 011 - 轻微右偏
            ChangeState(TRACKING_RIGHT_SLIGHT);
            break;
            
        case 0x04: // 只有L: 100 - 中等左偏
            ChangeState(TRACKING_LEFT_MODERATE);
            break;
            
        case 0x01: // 只有R: 001 - 中等右偏
            ChangeState(TRACKING_RIGHT_MODERATE);
            break;
            
        case 0x00: // 全白: 000 - 丢失路线
            ChangeState(TRACKING_LOST);
            break;
            
        case 0x07: // 全黑: 111 - 全黑状态（可能为十字路口）
            cross_start_time = Timer_GetSystemMillis();
            ChangeState(TRACKING_CROSS_START);
            break;
            
        default:
            SetMotorSpeed(speed_profile.straight_speed, speed_profile.straight_speed);
            break;
    }
}

/**
 * @brief 动态转向调整 - 三个传感器版本
 */
static void DynamicTurnAdjustment(uint8_t sensor_state)
{
    switch(current_state) {
        case TRACKING_LEFT_SLIGHT:
            if (sensor_state == 0x04) { // 只有L，升级为中等左转
                ChangeState(TRACKING_LEFT_MODERATE);
            }
            else if (sensor_state == 0x02) { // 回到C1，回到直行
                ChangeState(TRACKING_STRAIGHT);
            }
            break;
            
        case TRACKING_LEFT_MODERATE:
            if (sensor_state == 0x06 || sensor_state == 0x02) { // 回到L+C1或只有C1，降级
                ChangeState(TRACKING_LEFT_SLIGHT);
            }
            break;
            
        case TRACKING_RIGHT_SLIGHT:
            if (sensor_state == 0x01) { // 只有R，升级为中等右转
                ChangeState(TRACKING_RIGHT_MODERATE);
            }
            else if (sensor_state == 0x02) { // 回到C1，回到直行
                ChangeState(TRACKING_STRAIGHT);
            }
            break;
            
        case TRACKING_RIGHT_MODERATE:
            if (sensor_state == 0x03 || sensor_state == 0x02) { // 回到C1+R或只有C1，降级
                ChangeState(TRACKING_RIGHT_SLIGHT);
            }
            break;
            
        default:
            break;
    }
}

/**
 * @brief 处理循迹状态 - 简化版本，无时间累积
 */
static void HandleTrackingState(uint8_t sensor_state)
{
    // 根据当前状态设置差速转向
    switch(current_state) {
        case TRACKING_LEFT_SLIGHT:      // 轻微左转
            SetMotorSpeed(speed_profile.slight_turn_speed - BASE_SLIGHT_TURN_DIFF, 
                         speed_profile.slight_turn_speed + BASE_SLIGHT_TURN_DIFF);
            break;
            
        case TRACKING_RIGHT_SLIGHT:     // 轻微右转
            SetMotorSpeed(speed_profile.slight_turn_speed + BASE_SLIGHT_TURN_DIFF, 
                         speed_profile.slight_turn_speed - BASE_SLIGHT_TURN_DIFF);
            break;
            
        case TRACKING_LEFT_MODERATE:    // 中等左转
            SetMotorSpeed(speed_profile.moderate_turn_speed - BASE_MODERATE_TURN_DIFF, 
                         speed_profile.moderate_turn_speed + BASE_MODERATE_TURN_DIFF);
            break;
            
        case TRACKING_RIGHT_MODERATE:   // 中等右转
            SetMotorSpeed(speed_profile.moderate_turn_speed + BASE_MODERATE_TURN_DIFF, 
                         speed_profile.moderate_turn_speed - BASE_MODERATE_TURN_DIFF);
            break;
            
        case TRACKING_LEFT_SHARP:       // 急左转
            SetMotorSpeed(speed_profile.sharp_turn_speed - BASE_SHARP_TURN_DIFF, 
                         speed_profile.sharp_turn_speed + BASE_SHARP_TURN_DIFF);
            break;
            
        case TRACKING_RIGHT_SHARP:      // 急右转
            SetMotorSpeed(speed_profile.sharp_turn_speed + BASE_SHARP_TURN_DIFF, 
                         speed_profile.sharp_turn_speed - BASE_SHARP_TURN_DIFF);
            break;
            
        default:
            break;
    }
    
    // 检查状态转换条件
    if (sensor_state == 0x02) {         // 回到完美直行
        ChangeState(TRACKING_STRAIGHT);
    }
    else if (sensor_state == 0x00) {    // 丢失路线
        ChangeState(TRACKING_LOST);
    }
    else if (sensor_state == 0x07) {    // 进入十字路口（全黑）
        cross_start_time = Timer_GetSystemMillis();
        ChangeState(TRACKING_CROSS_START);
    }
    
    // 动态调整转向级别
    DynamicTurnAdjustment(sensor_state);
}

// 十字路口处理函数（简化）
static void HandleCrossStartState(uint8_t sensor_state)
{
    uint32_t current_time = Timer_GetSystemMillis();
    
    SetMotorSpeed(speed_profile.cross_speed, speed_profile.cross_speed);
    
    // 如果仍然全黑，继续前进
    if (sensor_state == 0x07) {
        // 保持十字路口状态
    }
    else if (current_time - cross_start_time > 200) {
        // 经过一段时间后，根据传感器状态回到循迹
        ChangeState(TRACKING_STRAIGHT);
    }
    else if (sensor_state != 0x07 && sensor_state != 0x00) {
        // 检测到路线，回到循迹
        ChangeState(TRACKING_STRAIGHT);
    }
}

static void HandleLostState(uint8_t sensor_state)
{
    // 根据最后的转向方向决定恢复方向
    if (current_state == TRACKING_LEFT_SLIGHT || current_state == TRACKING_LEFT_MODERATE || 
        current_state == TRACKING_LEFT_SHARP) {
        recovery_direction = 0; // 向左恢复
    } else {
        recovery_direction = 1; // 向右恢复
    }
    
    ChangeState(TRACKING_RECOVERING);
}

static void HandleRecoveringState(uint8_t sensor_state)
{
    uint32_t current_time = Timer_GetSystemMillis();
    
    // 根据恢复方向设置差速
    if (recovery_direction == 0) {
        SetMotorSpeed(RECOVERY_SPEED - 20, RECOVERY_SPEED + 20);
    } else {
        SetMotorSpeed(RECOVERY_SPEED + 20, RECOVERY_SPEED - 20);
    }
    
    // 如果重新检测到路线
    if (sensor_state != 0x00) {
        ChangeState(TRACKING_STRAIGHT);
    }
    // 如果超时仍未找到路线
    else if (current_time - state_start_time > RECOVERY_TIMEOUT) {
        SetMotorSpeed(0, 0); // 停止
    }
}

static void HandleAllBlackState(uint8_t sensor_state)
{
    SetMotorSpeed(speed_profile.straight_speed, speed_profile.straight_speed);
    
    // 如果不再全黑，回到循迹状态
    if (sensor_state != 0x07) {
        ChangeState(TRACKING_STRAIGHT);
    }
}

void TrackingStateMachine_Update(void)
{
    static uint32_t last_update_time = 0;
    uint32_t current_time = Timer_GetSystemMillis();
    
    if (current_time - last_update_time < STATE_UPDATE_INTERVAL) {
        return;
    }
    last_update_time = current_time;
    
    if (!IsAutoTrackingMode()) {
        if (current_state != TRACKING_READY) {
            ChangeState(TRACKING_READY);
            SetMotorSpeed(0, 0);
        }
        return;
    }
    
    uint8_t sensor_state = GetSensorState();
    system_status_packet.line_lost = (sensor_state == 0x00) ? 1 : 0;
    
    switch(current_state) {
        case TRACKING_READY:
            ChangeState(TRACKING_STRAIGHT);
            break;
        case TRACKING_STRAIGHT:
            HandleStraightState(sensor_state);
            break;
            
        // 转向状态
        case TRACKING_LEFT_SLIGHT:
        case TRACKING_RIGHT_SLIGHT:
        case TRACKING_LEFT_MODERATE:
        case TRACKING_RIGHT_MODERATE:
        case TRACKING_LEFT_SHARP:
        case TRACKING_RIGHT_SHARP:
            HandleTrackingState(sensor_state);
            break;
            
        case TRACKING_CROSS_START:
            HandleCrossStartState(sensor_state);
            break;
        case TRACKING_LOST:
            HandleLostState(sensor_state);
            break;
        case TRACKING_RECOVERING:
            HandleRecoveringState(sensor_state);
            break;
        case TRACKING_ALL_BLACK:
            HandleAllBlackState(sensor_state);
            break;
        case TRACKING_FINISH:
            SetMotorSpeed(0, 0);
            break;
        default:
            break;
    }
    
    last_sensor_state = sensor_state;
}

TrackingState_t Tracking_GetCurrentState(void)
{
    return current_state;
}
