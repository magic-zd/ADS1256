#ifndef __ADS1256_H
#define __ADS1256_H

#include "stm32f10x.h"

//===================== 第一片 ADS1256 (SPI2) =====================
#define ADS1256_1_CS_PIN        GPIO_Pin_12
#define ADS1256_1_CS_PORT       GPIOB
#define ADS1256_1_DRDY_PIN      GPIO_Pin_1
#define ADS1256_1_DRDY_PORT     GPIOB

//===================== 第二片 ADS1256 (SPI1) =====================
#define ADS1256_2_CS_PIN        GPIO_Pin_4   // PA4
#define ADS1256_2_CS_PORT       GPIOA
#define ADS1256_2_DRDY_PIN      GPIO_Pin_0   // Pb0
#define ADS1256_2_DRDY_PORT     GPIOB

//===================== 公共参数 =====================
#define VREF        5.0f
#define AVG_NUM     4

//===================== 函数声明 =====================
void ADS1256_InitAll(void);                // 初始化两片

// 读通道（1~4差分）
int32_t ADS1256_1_ReadChannel(uint8_t ch);
int32_t ADS1256_2_ReadChannel(uint8_t ch);

float ADS1256_ToVoltage(int32_t adc_val);
uint8_t ADS1256_1_ReadID(void);
uint8_t ADS1256_2_ReadID(void);
void ADS1256_1_Init(void);
void ADS1256_2_Init(void);
#endif
