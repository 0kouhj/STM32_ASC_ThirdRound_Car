#include "Infrared_Sensor.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "Sensor_Data.h"
Sensor_Packet sensor_packet = {
    .ir_sensor_raw = {0,0,0,0},         // 4个红外传感器原始值 [PA7, PA10, PA11, PA12]
    .left_encoder_raw = 0,              // 左编码器原始计数值
    .right_encoder_raw = 0,             // 右编码器原始计数值
    .battery_adc_raw = 0,               // 电池ADC原始值
};
// 传感器GPIO配置
typedef struct {
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
} SensorGPIO_t;

// 传感器引脚映射
static const SensorGPIO_t sensor_gpio[INFRARED_SENSOR_COUNT] = {
    {GPIOA, GPIO_Pin_7},   // 传感器1 - PA7
    {GPIOA, GPIO_Pin_10},  // 传感器2 - PA10
    {GPIOA, GPIO_Pin_11},  // 传感器3 - PA11
    {GPIOA, GPIO_Pin_12}   // 传感器4 - PA12
};

/**
  * @brief  红外传感器初始化
  */
void Infrared_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 开启GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    // 配置红外传感器引脚为输入模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入，默认高电平（白色）
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
  * @brief  更新所有传感器状态
  */
void Infrared_Update_SensorsState(void)
{
    for (uint8_t i = 0; i < INFRARED_SENSOR_COUNT; i++) {
        if (GPIO_ReadInputDataBit(sensor_gpio[i].GPIOx, sensor_gpio[i].GPIO_Pin) == Bit_RESET) {
            sensor_packet.ir_sensor_raw[i] = 0;  // 检测到黑线
        } else {
            sensor_packet.ir_sensor_raw[i] = 1;  // 检测到白线
        }                      
    }
}