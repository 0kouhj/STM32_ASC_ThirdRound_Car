#ifndef __TIMER_H
#define __TIMER_H

#include "stm32f10x.h"

// 计时器状态定义
typedef enum {
    TIMER_STOPPED = 0,
    TIMER_RUNNING,
    TIMER_PAUSED
} Timer_State_t;

// 计时器结构体
typedef struct {
    volatile uint32_t start_time;     // 开始时间
    volatile uint32_t pause_time;     // 暂停时的时间
    volatile uint32_t total_pause_time; // 总暂停时间
    Timer_State_t state;              // 计时器状态
} Timer_t;

// 函数声明
void Timer_Init(void);
void Timer_Start(Timer_t *timer);
void Timer_Stop(Timer_t *timer);
void Timer_Pause(Timer_t *timer);
void Timer_Resume(Timer_t *timer);
void Timer_Reset(Timer_t *timer);
void Timer_Update(void);
uint32_t Timer_GetElapsedTime(Timer_t *timer);
uint32_t Timer_GetCurrentTime(void);
uint8_t Timer_IsTimeout(Timer_t *timer, uint32_t timeout_ms);
uint32_t Timer_GetSystemMillis(void);

// 全局时间计数器（与您的millis_count保持一致）
extern volatile uint32_t system_millis;

#endif
