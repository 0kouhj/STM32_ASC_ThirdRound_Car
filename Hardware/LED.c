#include "LED.h"

/**
  * @brief  LED初始化
  * @param  无
  * @retval 无
  */
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 开启GPIO时钟
    RCC_APB2PeriphClockCmd(LED_GPIO_CLK, ENABLE);
    
    // 配置GPIO为推挽输出
    GPIO_InitStructure.GPIO_Pin = LED_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_GPIO_PORT, &GPIO_InitStructure);
    
    // 初始状态：LED熄灭
    LED_Off();
}

/**
  * @brief  点亮LED
  * @param  无
  * @retval 无
  */
void LED_On(void)
{
    GPIO_ResetBits(LED_GPIO_PORT, LED_GPIO_PIN);
}

/**
  * @brief  熄灭LED
  * @param  无
  * @retval 无
  */
void LED_Off(void)
{
    GPIO_SetBits(LED_GPIO_PORT, LED_GPIO_PIN);
}

/**
  * @brief  切换LED状态
  * @param  无
  * @retval 无
  */
void LED_Toggle(void)
{
    LED_GPIO_PORT->ODR ^= LED_GPIO_PIN;
}


