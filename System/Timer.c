#include "Timer.h"

// 全局毫秒计数器 - 与您现有的millis_count保持一致
volatile uint32_t system_millis = 0;

// 计时器分辨率（基于您的PWM配置计算）
// 您的配置：Period=100-1, Prescaler=36-1, 72MHz时钟
// 定时器频率 = 72MHz / 36 = 2MHz
// 中断频率 = 2MHz / 100 = 20kHz
// 中断周期 = 1/20kHz = 50us
#define TIMER_INTERRUPT_PERIOD_US 50  // 每次中断50us

// 毫秒计数器（每20次中断增加1ms）
static volatile uint16_t us_counter = 0;

/**
  * @brief  初始化计时系统（使用现有的TIM2配置）
  * @param  无
  * @retval 无
  */
void Timer_Init(void)
{
    // TIM2已经在PWM初始化中配置好了
    // 我们只需要确保中断已使能
    
    system_millis = 0;
    us_counter = 0;
}

/**
  * @brief  开始计时
  * @param  timer: 计时器指针
  * @retval 无
  */
void Timer_Start(Timer_t *timer)
{
    timer->state = TIMER_RUNNING;
    timer->start_time = system_millis;
    timer->total_pause_time = 0;
    timer->pause_time = 0;
}

/**
  * @brief  停止计时
  * @param  timer: 计时器指针
  * @retval 无
  */
void Timer_Stop(Timer_t *timer)
{
    timer->state = TIMER_STOPPED;
}

/**
  * @brief  暂停计时
  * @param  timer: 计时器指针
  * @retval 无
  */
void Timer_Pause(Timer_t *timer)
{
    if (timer->state == TIMER_RUNNING) {
        timer->state = TIMER_PAUSED;
        timer->pause_time = system_millis;
    }
}

/**
  * @brief  恢复计时
  * @param  timer: 计时器指针
  * @retval 无
  */
void Timer_Resume(Timer_t *timer)
{
    if (timer->state == TIMER_PAUSED) {
        timer->state = TIMER_RUNNING;
        timer->total_pause_time += (system_millis - timer->pause_time);
    }
}

/**
  * @brief  重置计时器
  * @param  timer: 计时器指针
  * @retval 无
  */
void Timer_Reset(Timer_t *timer)
{
    timer->state = TIMER_STOPPED;
    timer->start_time = 0;
    timer->pause_time = 0;
    timer->total_pause_time = 0;
}

/**
  * @brief  获取已过去的时间（毫秒）
  * @param  timer: 计时器指针
  * @retval 已过去的时间（毫秒）
  */
uint32_t Timer_GetElapsedTime(Timer_t *timer)
{
    if (timer->state == TIMER_STOPPED) {
        return 0;
    } else if (timer->state == TIMER_PAUSED) {
        return (timer->pause_time - timer->start_time - timer->total_pause_time);
    } else {
        return (system_millis - timer->start_time - timer->total_pause_time);
    }
}

/**
  * @brief  获取当前系统时间
  * @param  无
  * @retval 当前系统时间（毫秒）
  */
uint32_t Timer_GetCurrentTime(void)
{
    return system_millis;
}

/**
  * @brief  检查是否超时
  * @param  timer: 计时器指针
  * @param  timeout_ms: 超时时间（毫秒）
  * @retval 1: 超时, 0: 未超时
  */
uint8_t Timer_IsTimeout(Timer_t *timer, uint32_t timeout_ms)
{
    return (Timer_GetElapsedTime(timer) >= timeout_ms);
}

/**
  * @brief  获取系统毫秒数
  * @param  无
  * @retval 系统毫秒数
  */
uint32_t Timer_GetSystemMillis(void)
{
    return system_millis;
}

/**
  * @brief  更新系统时间（在TIM2中断中调用）
  * @param  无
  * @retval 无
  */
void Timer_Update(void)
{
    us_counter += TIMER_INTERRUPT_PERIOD_US;
    if (us_counter >= 1000) {  // 达到1ms
        system_millis++;
        us_counter -= 1000;
    }
}
