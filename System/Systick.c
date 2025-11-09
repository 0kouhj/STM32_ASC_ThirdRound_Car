#include "stm32f10x.h"
#include "Systick.h"
#include "Serial.h"
volatile uint32_t millis_count = 0;

void SysTick_Init(void)
{
    // 确保SystemCoreClock已正确设置
    SystemCoreClockUpdate(); // 添加这行来更新时钟频率
    
    // 配置SysTick为1ms中断
    // 对于72MHz的系统时钟，重装载值应该是72000-1
    if (SysTick_Config(SystemCoreClock / 1000)) {
        // 初始化错误处理
        Serial_SendString("@ERROR: SysTick Config Failed!\r\n");
        while (1);
    }
    
    // 设置中断优先级
    NVIC_SetPriority(SysTick_IRQn, 0);
    
    Serial_Printf("@Systick: Clock=%lu Hz, Reload=%lu\r\n", 
                  SystemCoreClock, SystemCoreClock/1000);
}

uint32_t get_millis(void)
{
    return millis_count;
}

// SysTick中断处理函数
void SysTick_Handler(void)
{
    millis_count++;
}
