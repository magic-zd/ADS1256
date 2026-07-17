

//单片机型号：STC15W408AS 
//	电压	：5V 
//内部RC振荡：11.0592M



#include <stdio.h>
#include "STC15F2K60S2.H"
#include "ADS1256.H"

sbit LED = P5^5;


#define u8 unsigned char 
#define u16 unsigned short
							
//	sbit SCK  = P1^2;
//	sbit DIN  = P1^3;
//	sbit DOUT = P1^4; 
//	sbit DRDY = P1^5;	
//	sbit CS   = P1^6;	 
						  
sbit LCD_EN=P2^7;
sbit LCD_RW=P2^6;
sbit LCD_RS=P2^5;

#define LINE1_COLUMN(x) 0x7f+x
#define LINE2_COLUMN(x) 0xbf+x

#define LCD_DATAPORT P0

void DelayMs( unsigned int ms)
{
	unsigned int i,j;
	for (i = 0; i < ms; i++)
		for(j=0;j<115;j++);
}

void delay_ms(u16 nms)
{
	u16 x,y;
	for(x=nms;x>0;x--)
		for(y=120;y>0;y--);
}

void LCD1602_WriteCMD(u8 cmd)//LCD1602写命令函数
{
	 LCD_RS=0;
	 LCD_DATAPORT=cmd;
	 delay_ms(1);
	 LCD_EN=1;
	 delay_ms(1);
	 LCD_EN=0;
}

void LCD1602_WriteData(u8 dat)//LCD1602写数据函数
{
	 LCD_RS=1;
	 LCD_DATAPORT=dat;
	 delay_ms(1);
	 LCD_EN=1;
	 delay_ms(1);
	 LCD_EN=0;
}

void display_char(u8 addr,u8 chr)
{
	LCD1602_WriteCMD(addr);
	LCD1602_WriteData(chr);
}



void display_num4(u8 addr,u16 num)
{
	u8 qian,bai,shi,ge;
	
	qian= num/1000;
	bai = num/100%10;
	shi = num/10%10;
	ge = num%10;			//变量分离
	display_char(addr,qian+'0');
	display_char(addr+1,'.');
	display_char(addr+2,bai+'0');
	display_char(addr+3,shi+'0');
	display_char(addr+4,ge+'0');
}


void LCD_Init(void)
{
	LCD_EN=0;//打开片选
	LCD_RW=0;//读写控制，1：读	
			//			 0：写	 这里只写，不读
	LCD1602_WriteCMD(0x38);
	LCD1602_WriteCMD(0x0e);
	LCD1602_WriteCMD(0x06);//地址指针自动加一，且光标加一
	LCD1602_WriteCMD(0x01);
	LCD1602_WriteCMD(0x80);
}



void main()
{
	signed	long Adc;
	float  Volts;

	DelayMs(50);
  ADS1256_Init();			//ADS1256 参数初始化
	LCD_Init();
	while(1)
	{
		Adc = ADS1256ReadData( ADS1256_MUXP_AIN0|ADS1256_MUXN_AINCOM);	
		Volts = Adc*0.00059453; // 0.00000059453 为系数，ADC生产出后都有一定的偏差，在此校准。
		display_num4(LINE1_COLUMN(1),Volts);
		delay_ms(100);
		
		Adc = ADS1256ReadData( ADS1256_MUXP_AIN1|ADS1256_MUXN_AINCOM);	
		Volts = Adc*0.00059453; // 0.00000059453 为系数，ADC生产出后都有一定的偏差，在此校准。
		display_num4(LINE1_COLUMN(8),Volts);
		delay_ms(100);
		
		Adc = ADS1256ReadData( ADS1256_MUXP_AIN2|ADS1256_MUXN_AIN3);	
		Volts = Adc*0.00059453; // 0.00000059453 为系数，ADC生产出后都有一定的偏差，在此校准。
		display_num4(LINE2_COLUMN(1),Volts);
		delay_ms(100);
		
	    Adc = ADS1256ReadData( ADS1256_MUXP_AIN3|ADS1256_MUXN_AINCOM);	
		Volts = Adc*0.00059453; // 0.00000059453 为系数，ADC生产出后都有一定的偏差，在此校准。
		display_num4(LINE2_COLUMN(8),Volts);
		
		DelayMs(1000);
	}
}




