#include "ads1256.h"
#include "delay.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_gpio.h"




// SPI 读写（保持不变）
uint8_t ADS1256_SPI_RW(uint8_t dat)
{
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI2, dat);
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI2);
}

// SPI 初始化（保持不变）
void ADS1256_SPI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    // SCK(PB13) / MOSI(PB15)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // MISO(PB14)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // CS(PB12)
    GPIO_InitStructure.GPIO_Pin = ADS1256_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ADS1256_CS_PORT, &GPIO_InitStructure);
    GPIO_SetBits(ADS1256_CS_PORT, ADS1256_CS_PIN);

    // SPI 模式1（符合手册要求）
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

// 等待 DRDY 低电平（增加超时）
void ADS1256_WaitDRDY(void)
{
 
    while(GPIO_ReadInputDataBit(ADS1256_DRDY_PORT, ADS1256_DRDY_PIN) != 0);

}

// 写入单个寄存器
static void ADS1256_WriteReg(uint8_t reg_addr, uint8_t reg_val)
{
    GPIO_ResetBits(ADS1256_CS_PORT, ADS1256_CS_PIN);
    ADS1256_WaitDRDY();
		ADS1256_SPI_RW(0x50 | reg_addr); // WREG命令 + 寄存器地址
    ADS1256_SPI_RW(0x00);           // 写入1个寄存器（数量-1=0）
    ADS1256_SPI_RW(reg_val);        // 写入数据
    GPIO_SetBits(ADS1256_CS_PORT, ADS1256_CS_PIN);
    Delay_ms(1);                    // 确保配置生效
}

// 执行自校准
static void ADS1256_SelfCal(void)
{
    ADS1256_WaitDRDY();
    GPIO_ResetBits(ADS1256_CS_PORT, ADS1256_CS_PIN);
    ADS1256_SPI_RW(0xF0);           // SELFCAL命令（偏移+增益校准）
    ADS1256_WaitDRDY();             // 等待校准完成
		GPIO_SetBits(ADS1256_CS_PORT, ADS1256_CS_PIN);
}

// ADC 初始化（核心修改：关闭Buffer）
void ADS1256_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    ADS1256_SPI_Init();
    Delay_ms(100);

    // RESET引脚配置（PB0）
    GPIO_InitStructure.GPIO_Pin = ADS1256_RESET_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ADS1256_RESET_PORT, &GPIO_InitStructure);

    // DRDY引脚配置（PB1）
    GPIO_InitStructure.GPIO_Pin = ADS1256_DRDY_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(ADS1256_DRDY_PORT, &GPIO_InitStructure);

//    // 硬件复位
//    GPIO_ResetBits(ADS1256_RESET_PORT, ADS1256_RESET_PIN);
//    Delay_ms(10);
//    GPIO_SetBits(ADS1256_RESET_PORT, ADS1256_RESET_PIN);
//    Delay_ms(100);
    ADS1256_WaitDRDY();

    // 关键配置：关闭Buffer（BUFEN=0），其他参数保持兼容
    ADS1256_WriteReg(0x00, 0x04);   // STATUS寄存器：BUFEN=0（关闭缓冲）  04
    ADS1256_WriteReg(0x02, 0x00);   // ADCON寄存器：PGA=1 
    ADS1256_WriteReg(0x03, 0x03);   // DRATE寄存器：2.5SPS（低噪声）

    // 执行自校准-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    ADS1256_SelfCal();
}

// 读取指定通道
int32_t ADS1256_ReadChannel(uint8_t ch)
{
    uint8_t buf[3];
    int32_t val;
    uint8_t i;
    int32_t sum = 0;

    if(ch > 7) return 0;

    for(i = 0; i < AVG_NUM; i++)
    {
        // 配置通道（MUX寄存器）
        ADS1256_WaitDRDY();
 
        ADS1256_SPI_RW(0x51);                          // WREG + 地址01h（MUX）
        ADS1256_SPI_RW(0x00);                          // 写入1个寄存器
			  ADS1256_SPI_RW(0x08);                          // MUX 
//        ADS1256_SPI_RW((ch << 4) | 0x08);              // 单端输入：AINch - AINCOM
			
                               // 通道稳定时间
				GPIO_ResetBits(ADS1256_CS_PORT, ADS1256_CS_PIN);

				ADS1256_SPI_RW(0xFC);  //sync
//				Delay_us(1);  // 满足t11时序（4×τCLKIN≈520ns）
				ADS1256_SPI_RW(0x00);   //wakeup
				
			
        ADS1256_SPI_RW(0x01);                          // RDATA命令
        Delay_us(10);                                  // 满足t6时序
        buf[0] = ADS1256_SPI_RW(0xFF);                  // 高位字节
        buf[1] = ADS1256_SPI_RW(0xFF);                  // 中位字节
        buf[2] = ADS1256_SPI_RW(0xFF);                  // 低位字节
        GPIO_SetBits(ADS1256_CS_PORT, ADS1256_CS_PIN);

        // 数据拼接
        val = ((int32_t)buf[0] << 16) | ((uint16_t)buf[1] << 8) | buf[2];
        if(val & 0x800000) val |= 0xFF000000;           // 符号位扩展

        sum += val;
    }
		GPIO_SetBits(ADS1256_CS_PORT, ADS1256_CS_PIN);
    return sum / AVG_NUM;
}

// 转换为电压
float ADS1256_ToVoltage(int32_t adc_val)
{
    return (float)adc_val * VREF / 8388607.0f;
}

// 读取芯片ID（修正后）
uint8_t ADS1256_ReadID(void)
{
    uint8_t id;

    ADS1256_WaitDRDY();
    GPIO_ResetBits(ADS1256_CS_PORT, ADS1256_CS_PIN);
    ADS1256_SPI_RW(0x10);           // RREG + 地址0x00（STATUS寄存器）
    ADS1256_SPI_RW(0x00);           // 读取1个寄存器
    Delay_us(10);
    id = ADS1256_SPI_RW(0xFF);      // 读取数据
    GPIO_SetBits(ADS1256_CS_PORT, ADS1256_CS_PIN);
    id = id >> 4;                   // 提取高4位ID（正确值=3）

    return id;
}

// 读取寄存器（调试用）
uint8_t ADS1256_ReadReg(uint8_t reg_addr)
{
    uint8_t val;
    ADS1256_WaitDRDY();
    GPIO_ResetBits(ADS1256_CS_PORT, ADS1256_CS_PIN);
    ADS1256_SPI_RW(0x10 | reg_addr); // RREG命令 + 地址
    ADS1256_SPI_RW(0x00);           // 读取1个寄存器
    Delay_us(10);
    val = ADS1256_SPI_RW(0xFF);
    GPIO_SetBits(ADS1256_CS_PORT, ADS1256_CS_PIN);
    return val;
}
