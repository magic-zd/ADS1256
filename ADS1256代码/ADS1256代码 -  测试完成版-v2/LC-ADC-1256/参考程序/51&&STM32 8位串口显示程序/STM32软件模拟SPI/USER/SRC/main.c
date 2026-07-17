#include "stm32f10x.h"
#include "systemconfig.h" 
#include "common.h"
#include <stdio.h>
#include "ads1256.h"

#define LED1	BIT_ADDR(GPIOB_ODR_Addr,0)
#define LED2	BIT_ADDR(GPIOB_ODR_Addr,1)
#define KEY		BIT_ADDR(GPIOB_IDR_Addr,2)



//PA9  -----> Txd1
//PA10 -----> Rxd1


//模拟SPI，模块和开发板依次相连
//***************************
//		Pin assign	   	
//			STM32			ADS1256
//		GPIOB_Pin_15 		---> SCK
//		GPIOB_Pin_14		---> DIN
//		GPIOB_Pin_13  		<--- DOUT
//		GPIOB_Pin_12 		<--- DRDY
//		GPIOB_Pin_11 		---> CS

//***************************	
int main(void)
{	

	u8 i=0;
	int Adc;
	float Volts;


	SystemConfiguration();		    //系统初始化
	USART_Config(USART1,115200);    //串口1初始化，波特率 115200
	Init_ADS1256_GPIO(); //初始化ADS1256 GPIO管脚 
	Delay_ms(50);
	ADS1256_Init();

	while(1)
	{	

		LED2 = !LED2_READ;	//指示灯闪烁
		for(i = 0;i < 8;i++)
		{

			Adc = ADS1256ReadData( (i << 4) | ADS1256_MUXN_AINCOM);// 相当于 ( ADS1256_MUXP_AIN0 | ADS1256_MUXN_AINCOM);		

			 /*差分采集方式*/
			//Adc = ADS1256ReadData( ADS1256_MUXP_AIN0|ADS1256_MUXN_AIN1); //P = AIN0 ,N = AIN1 差分方式*/
			Volts = Adc*0.000000598;

			printf(" %.4fV  ",Volts);
			
		}
		//Delay_ms(100);		
		printf("\r\n"); 
	}

}




