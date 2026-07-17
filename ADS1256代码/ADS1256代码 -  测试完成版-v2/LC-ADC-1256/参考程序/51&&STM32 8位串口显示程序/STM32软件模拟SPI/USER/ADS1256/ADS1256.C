
#include <stdio.h>
#include "stm32f10x_gpio.h"
#include "ADS1256.h"

//依次相连
//***************************
//		Pin assign	   	
//			STM32			ADS1256

//		GPIOB_Pin_15 		---> SCK
//		GPIOB_Pin_14		---> DIN
//		GPIOB_Pin_13  		<--- DOUT
//		GPIOB_Pin_12 		<--- DRDY
//		GPIOB_Pin_11 		---> CS

//***************************	

	/*端口定义*/ 

 //	#define RCC_SCK 	RCC_APB2Periph_GPIOB //SCK
	#define PORT_SCK	GPIOB
	#define PIN_SCK		GPIO_Pin_15

//	#define RCC_DOUT 	RCC_APB2Periph_GPIOB //MOSI
	#define PORT_DOUT	GPIOB
	#define PIN_DOUT	GPIO_Pin_14

//	#define RCC_DIN 	RCC_APB2Periph_GPIOB //MISO
	#define PORT_DIN	GPIOB
	#define PIN_DIN		GPIO_Pin_13

//	#define RCC_DRDY 	RCC_APB2Periph_GPIOB //DRDY
	#define PORT_DRDY	GPIOB
	#define PIN_DRDY	GPIO_Pin_12

//	#define RCC_CS 		RCC_APB2Periph_GPIOB //CS
	#define PORT_CS		GPIOB
	#define PIN_CS		GPIO_Pin_11


//  #define RCC_PWDN 	RCC_APB2Periph_GPIOB //PWDN
//	#define PORT_PWDN	GPIOB
//	#define PIN_PWDN	GPIO_Pin_10

//****************************************************************


	#define CS_0()		GPIO_ResetBits(PORT_CS, PIN_CS)
	#define CS_1()		GPIO_SetBits(PORT_CS, PIN_CS)
	#define SCK_0()		GPIO_ResetBits(PORT_SCK, PIN_SCK)
	#define SCK_1()		GPIO_SetBits(PORT_SCK, PIN_SCK)

	#define ADS1256_DIN_0()		GPIO_ResetBits(PORT_DOUT, PIN_DOUT)		//Set ADS1256_DIN = 0
	#define ADS1256_DIN_1()		GPIO_SetBits(PORT_DOUT, PIN_DOUT)		//Set ADS1256_DIN = 1

	#define ADS1256_DRDY  (PORT_DRDY->IDR & PIN_DRDY)
	#define ADS1256_DOUT   (PORT_DIN->IDR & PIN_DIN)



//初始化ADS1256 GPIO
void Init_ADS1256_GPIO(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = PIN_DRDY|PIN_DIN; 
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  	GPIO_Init(PORT_DRDY, &GPIO_InitStructure);

	CS_1();
    GPIO_InitStructure.GPIO_Pin = 	 PIN_SCK|PIN_CS|PIN_DOUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(PORT_SCK, &GPIO_InitStructure);
	 
	
}



static void ADS1256_DelaySCLK(void)
{
	uint16_t i;

	/*
		取 5 时，实测高电平200ns, 低电平250ns <-- 不稳定
		取 10 以上，可以正常工作， 低电平400ns 高定400ns <--- 稳定
	*/
	for (i = 0; i < 15; i++);
}
/*
*********************************************************************************************************
*	函 数 名: SPI_WriteByte
*	功能说明: 向SPI总线发送8个bit数据。 不带CS控制。
*	形    参: _data : 数据
*	返 回 值: 无
*********************************************************************************************************
*/
void SPI_WriteByte(unsigned char TxData)
{
	unsigned char i;
	/* 连续发送多个字节时，需要延迟一下 */
	ADS1256_DelaySCLK();
	ADS1256_DelaySCLK();
	/*　ADS1256 要求 SCL高电平和低电平持续时间最小 200ns  */
	for(i = 0; i < 8; i++)
	{
		if (TxData & 0x80)
			ADS1256_DIN_1();
		else
			ADS1256_DIN_0();
		
		SCK_1();
		ADS1256_DelaySCLK();
		TxData <<= 1;
		SCK_0();			/* <----  ADS1256 是在SCK下降沿采样DIN数据, 数据必须维持 50nS */
		ADS1256_DelaySCLK();
	}
	
} 

/*
*********************************************************************************************************
*	函 数 名: SPI_ReadByte
*	功能说明: 从SPI总线接收8个bit数据。 不带CS控制。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
unsigned char SPI_ReadByte(void)
{
	unsigned char i;
	unsigned char read = 0;
	ADS1256_DelaySCLK();
	/*　ADS1256 要求 SCL高电平和低电平持续时间最小 200ns  */
	for (i = 0; i < 8; i++)
	{
		SCK_1();
		ADS1256_DelaySCLK();
		read = read<<1;
		SCK_0();
		if (ADS1256_DOUT)
		{
			read++;
		}
		ADS1256_DelaySCLK();
	}
	return read;
}

//-----------------------------------------------------------------//
//	功    能：ADS1256 写数据
//	入口参数: /
//	出口参数: /
//	全局变量: /
//	备    注: 向ADS1256中地址为regaddr的寄存器写入一个字节databyte
//-----------------------------------------------------------------//
void ADS1256WREG(unsigned char regaddr,unsigned char databyte)
{

	CS_0();
	while(ADS1256_DRDY);//当ADS1256_DRDY为低时才能写寄存器
	//向寄存器写入数据地址
    SPI_WriteByte(ADS1256_CMD_WREG | (regaddr & 0x0F));
    //写入数据的个数n-1
    SPI_WriteByte(0x00);
    //向regaddr地址指向的寄存器写入数据databyte
    SPI_WriteByte(databyte);
	CS_1();


}
//初始化ADS1256
void ADS1256_Init(void)
{
	//*************自校准****************
   	while(ADS1256_DRDY);
	CS_0();
	SPI_WriteByte(ADS1256_CMD_SELFCAL);
	while(ADS1256_DRDY);
	CS_1();
	//**********************************

	ADS1256WREG(ADS1256_STATUS,0x06);               // 高位在前、校准、使用缓冲
	//ADS1256WREG(ADS1256_STATUS,0x04);               // 高位在前、不使用缓冲
	//ADS1256WREG(ADS1256_MUX,0x08);                  // 初始化端口A0为‘+’，AINCOM位‘-’

	ADS1256WREG(ADS1256_ADCON,ADS1256_GAIN_1);      // 放大倍数1
	ADS1256WREG(ADS1256_DRATE,ADS1256_DRATE_10SPS); // 数据10sps
	ADS1256WREG(ADS1256_IO,0x00);  
	
	//*************自校准****************
	while(ADS1256_DRDY);
	CS_0();
	SPI_WriteByte(ADS1256_CMD_SELFCAL);
	while(ADS1256_DRDY);
	CS_1(); 
	//**********************************
}             


//读取AD值
signed int ADS1256ReadData(unsigned char channel)  
{

    unsigned int sum=0;

	while(ADS1256_DRDY);//当ADS1256_DRDY为低时才能写寄存器 
	ADS1256WREG(ADS1256_MUX,channel);		//设置通道
	CS_0();
	SPI_WriteByte(ADS1256_CMD_SYNC);
	SPI_WriteByte(ADS1256_CMD_WAKEUP);	               
	SPI_WriteByte(ADS1256_CMD_RDATA);
   	sum |= (SPI_ReadByte() << 16);
	sum |= (SPI_ReadByte() << 8);
	sum |= SPI_ReadByte();
	CS_1();

	if (sum>0x7FFFFF)           // if MSB=1, 
	{
		sum -= 0x1000000;       // do 2's complement

	}
    return sum;
}



