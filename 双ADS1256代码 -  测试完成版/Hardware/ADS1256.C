#include "ads1256.h"
#include "delay.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_gpio.h"
#include "usart.h"
#include <stdio.h>

//================================================================
//                      SPI2 （第一片 ADS1256）
//================================================================
uint8_t ADS1256_1_SPI_RW(uint8_t dat)
{
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI2, dat);
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI2);
}

void ADS1256_1_SPI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    // SCK PB13  MOSI PB15
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // MISO PB14
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // CS PB12
    GPIO_InitStructure.GPIO_Pin = ADS1256_1_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ADS1256_1_CS_PORT, &GPIO_InitStructure);
    GPIO_SetBits(ADS1256_1_CS_PORT, ADS1256_1_CS_PIN);

    // SPI 模式1
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI2, &SPI_InitStructure);
    SPI_Cmd(SPI2, ENABLE);
}

void ADS1256_1_WaitDRDY(void)
{
    while(GPIO_ReadInputDataBit(ADS1256_1_DRDY_PORT, ADS1256_1_DRDY_PIN) != 0);
}

static void ADS1256_1_WriteReg(uint8_t reg_addr, uint8_t reg_val)
{
    GPIO_ResetBits(ADS1256_1_CS_PORT, ADS1256_1_CS_PIN);
    ADS1256_1_WaitDRDY();
    ADS1256_1_SPI_RW(0x50 | reg_addr);
    ADS1256_1_SPI_RW(0x00);
    ADS1256_1_SPI_RW(reg_val);
    GPIO_SetBits(ADS1256_1_CS_PORT, ADS1256_1_CS_PIN);
    Delay_ms(1);
}

static void ADS1256_1_SelfCal(void)
{
    ADS1256_1_WaitDRDY();
    GPIO_ResetBits(ADS1256_1_CS_PORT, ADS1256_1_CS_PIN);
    ADS1256_1_SPI_RW(0xF0);
    ADS1256_1_WaitDRDY();
    GPIO_SetBits(ADS1256_1_CS_PORT, ADS1256_1_CS_PIN);
}

void ADS1256_1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADS1256_1_SPI_Init();

    Delay_ms(100);

    // DRDY PB1
    GPIO_InitStructure.GPIO_Pin = ADS1256_1_DRDY_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ADS1256_1_DRDY_PORT, &GPIO_InitStructure);

    ADS1256_1_WaitDRDY();
    ADS1256_1_WriteReg(0x00, 0x04);
    ADS1256_1_WriteReg(0x02, 0x00);
    ADS1256_1_WriteReg(0x03, 0x03);
		
    ADS1256_1_SelfCal();
}

int32_t ADS1256_1_ReadChannel(uint8_t ch)
{
    uint8_t buf[3]={0};
    int32_t val=0, sum=0;
    uint8_t i;
    if(ch>4) return 0;

    for(i=0; i<AVG_NUM+1; i++)
    {
        ADS1256_1_WaitDRDY();
        int j=(ch-1)*2;
        int k=(ch-1)*2+1;
        ADS1256_1_WriteReg(0x01, (j<<4)|k);
        Delay_ms(10);

        GPIO_ResetBits(ADS1256_1_CS_PORT, ADS1256_1_CS_PIN);
        ADS1256_1_SPI_RW(0xFC);
        ADS1256_1_SPI_RW(0x00);
        ADS1256_1_SPI_RW(0x01);
        Delay_us(10);

        buf[0]=ADS1256_1_SPI_RW(0xFF);
        buf[1]=ADS1256_1_SPI_RW(0xFF);
        buf[2]=ADS1256_1_SPI_RW(0xFF);
        GPIO_SetBits(ADS1256_1_CS_PORT, ADS1256_1_CS_PIN);

        val=((int32_t)buf[0]<<16)|(buf[1]<<8)|buf[2];
        if(i==0) val=0;
        if(val&0x800000) val |=0xFF000000;
        sum+=val;
        val=0;
    }
    return sum/AVG_NUM;
}

uint8_t ADS1256_1_ReadID(void)
{
    uint8_t id;
    ADS1256_1_WaitDRDY();
    GPIO_ResetBits(ADS1256_1_CS_PORT, ADS1256_1_CS_PIN);
    ADS1256_1_SPI_RW(0x10);
    ADS1256_1_SPI_RW(0x00);
    Delay_us(10);
    id=ADS1256_1_SPI_RW(0xFF);
    GPIO_SetBits(ADS1256_1_CS_PORT, ADS1256_1_CS_PIN);
    return id>>4;
}

//================================================================
//                      SPI1 （第二片 ADS1256）
//================================================================
uint8_t ADS1256_2_SPI_RW(uint8_t dat)
{
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, dat);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI1);
}

void ADS1256_2_SPI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA, ENABLE);

    // SCK PA5  MOSI PA7
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // MISO PA6
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // CS PA4
    GPIO_InitStructure.GPIO_Pin = ADS1256_2_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ADS1256_2_CS_PORT, &GPIO_InitStructure);
    GPIO_SetBits(ADS1256_2_CS_PORT, ADS1256_2_CS_PIN);

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_Cmd(SPI1, ENABLE);
}

void ADS1256_2_WaitDRDY(void)
{
    while(GPIO_ReadInputDataBit(ADS1256_2_DRDY_PORT, ADS1256_2_DRDY_PIN) != 0);
}

static void ADS1256_2_WriteReg(uint8_t reg_addr, uint8_t reg_val)
{
    GPIO_ResetBits(ADS1256_2_CS_PORT, ADS1256_2_CS_PIN);
    ADS1256_2_WaitDRDY();
    ADS1256_2_SPI_RW(0x50 | reg_addr);
    ADS1256_2_SPI_RW(0x00);
    ADS1256_2_SPI_RW(reg_val);
    GPIO_SetBits(ADS1256_2_CS_PORT, ADS1256_2_CS_PIN);
    Delay_ms(1);
}

static void ADS1256_2_SelfCal(void)
{
    ADS1256_2_WaitDRDY();
    GPIO_ResetBits(ADS1256_2_CS_PORT, ADS1256_2_CS_PIN);
    ADS1256_2_SPI_RW(0xF0);
    ADS1256_2_WaitDRDY();
    GPIO_SetBits(ADS1256_2_CS_PORT, ADS1256_2_CS_PIN);
}

void ADS1256_2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADS1256_2_SPI_Init();
    Delay_ms(100);                                 /////////////
		
    // DRDY Pb0
    GPIO_InitStructure.GPIO_Pin = ADS1256_2_DRDY_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ADS1256_2_DRDY_PORT, &GPIO_InitStructure);

    ADS1256_2_WaitDRDY();
    ADS1256_2_WriteReg(0x00, 0x04);
    ADS1256_2_WriteReg(0x02, 0x00);
    ADS1256_2_WriteReg(0x03, 0x03);
	                              ////////////////
    ADS1256_2_SelfCal();
}

int32_t ADS1256_2_ReadChannel(uint8_t ch)
{
    uint8_t buf[3]={0};
    int32_t val=0, sum=0;
    uint8_t i;
    if(ch>4) return 0;

    for(i=0; i<AVG_NUM+1; i++)
    {
        ADS1256_2_WaitDRDY();
        int j=(ch-1)*2;
        int k=(ch-1)*2+1;
        ADS1256_2_WriteReg(0x01, (j<<4)|k);
        Delay_ms(10);

        GPIO_ResetBits(ADS1256_2_CS_PORT, ADS1256_2_CS_PIN);
        ADS1256_2_SPI_RW(0xFC);
        ADS1256_2_SPI_RW(0x00);
        ADS1256_2_SPI_RW(0x01);
        Delay_us(10);

        buf[0]=ADS1256_2_SPI_RW(0xFF);
        buf[1]=ADS1256_2_SPI_RW(0xFF);
        buf[2]=ADS1256_2_SPI_RW(0xFF);
        GPIO_SetBits(ADS1256_2_CS_PORT, ADS1256_2_CS_PIN);

        val=((int32_t)buf[0]<<16)|(buf[1]<<8)|buf[2];
        if(i==0) val=0;
        if(val&0x800000) val |=0xFF000000;
        sum+=val;
        val=0;
    }
    return sum/AVG_NUM;
}

uint8_t ADS1256_2_ReadID(void)
{
    uint8_t id;
    ADS1256_2_WaitDRDY();
    GPIO_ResetBits(ADS1256_2_CS_PORT, ADS1256_2_CS_PIN);
    ADS1256_2_SPI_RW(0x10);
    ADS1256_2_SPI_RW(0x00);
    Delay_us(10);
    id=ADS1256_2_SPI_RW(0xFF);
    GPIO_SetBits(ADS1256_2_CS_PORT, ADS1256_2_CS_PIN);
    return id>>4;
}

//================================================================
//                      统一初始化两片
//================================================================
void ADS1256_InitAll(void)
{
    ADS1256_1_Init();
    ADS1256_2_Init();
}

//================================================================
//                      电压转换（公共）
//================================================================
float ADS1256_ToVoltage(int32_t adc_val)
{
    return (float)adc_val * VREF / 8388607.0f;
}
