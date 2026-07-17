#ifndef __ADS1256_H
#define __ADS1256_H

#include "stm32f10x.h"

// 引脚定义
#define ADS1256_CS_PIN        GPIO_Pin_12
#define ADS1256_CS_PORT       GPIOB
#define ADS1256_DRDY_PIN      GPIO_Pin_1//   a2是板子画错临时改动 正确的应该是b1
#define ADS1256_DRDY_PORT     GPIOB
//#define ADS1256_RESET_PIN     GPIO_Pin_0
//#define ADS1256_RESET_PORT    GPIOB
#define VREF 5.0f  // 参考电压（AVDD=5V）
#define AVG_NUM 4  // 采样平均次数

// 函数声明
void ADS1256_Init(void);
void ADS1256_WaitDRDY(void);
int32_t ADS1256_ReadChannel(uint8_t ch);
float ADS1256_ToVoltage(int32_t adc_val);
uint8_t ADS1256_ReadReg(uint8_t reg_addr);


// void ADS1256_SPI_Init(void);
uint8_t ADS1256_ReadID(void);  // 读取芯片ID
#endif
