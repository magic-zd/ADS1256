

//单片机型号：STC15W408AS 
//	电压	：5V 
//内部RC振荡：11.0592M



#include <stdio.h>
#include "STC15F2K60S2.H"
#include "ADS1256.H"
#include "uart.h"

sbit LED = P5^5;

/*
*********************************************************************************************************
*	函 数 名: DelayMs
*	功能说明: 时钟延时
*	形    参: ms 以毫秒为单位.
*	返 回 值: 无
*********************************************************************************************************
*/
void DelayMs( unsigned int ms)
{
	unsigned int i,j;
	for (i = 0; i < ms; i++)
		for(j=0;j<1074;j++);
}

#define u8 unsigned char 
#define u16 unsigned short
							
//	sbit SCK  = P1^2;
//	sbit DIN  = P1^3;
//	sbit DOUT = P1^4; 
//	sbit DRDY = P1^5;	
//	sbit CS   = P1^6;	 
						  

void main()
{

	u16 ch=0;
	signed	long Adc;
	float  Volts;
	char str[20];

    InitCOM();  			//初始化串口  波特率 9600
	Init_ADS1256_GPIO();  	// 配置I/O口
	DelayMs(50);
    ADS1256_Init();			//ADS1256 参数初始化

	while(1)
	{


		for(ch = 0;ch < 8;ch++)
		{	
			//Adc = ADS1256ReadData((u8)( 0| ADS1256_MUXN_AINCOM));// 相当于 ( ADS1256_MUXP_AIN0 | ADS1256_MUXN_AINCOM);		
			Adc = ADS1256ReadData( ch<<4|ADS1256_MUXN_AINCOM);
				
			 /*差分采集方式*/
			 /*Adc = ADS1256ReadData( ADS1256_MUXP_AIN0|ADS1256_MUXN_AIN1); //P = AIN0 ,N = AIN1 差分方式*/
			
			Volts = Adc*.00000059453; // 0.00000059453 为系数，ADC生产出后都有一定的偏差，在此校准。
			sprintf(str,"通道%d:%fV ",(u16)ch,Volts);  // 最好把串口调试助手窗口放到最大，数据会自动排列整齐，看起来方便些。	
			SendString(str);


		}
	 	 	SendString("\r\n");
			DelayMs(10);
		   	LED=!LED;
	}

	
}




