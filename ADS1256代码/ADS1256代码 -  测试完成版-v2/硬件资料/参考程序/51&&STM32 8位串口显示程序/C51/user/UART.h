//////////////////////////////////
//


#include "STC15F2K60S2.H"
#include <string.h>
#include <intrins.h>

#define FOSC 11059200L          //系统频率
#define BAUD 9600             //串口波特率



typedef unsigned char BYTE;
typedef unsigned int WORD;

bit busy;



void InitCOM(void)
{


    SCON = 0x50;                //8位可变波特率
    T2L = (65536 - (FOSC/4/BAUD));   //设置波特率重装值
    T2H = (65536 - (FOSC/4/BAUD))>>8;
    AUXR = 0x14;                //T2为1T模式, 并启动定时器2
    AUXR |= 0x01;               //选择定时器2为串口1的波特率发生器
    ES = 1;                     //使能串口1中断
    EA = 1;

}

/*----------------------------
发送串口数据
----------------------------*/
void SendData(BYTE dat)
{
    while (busy);               //等待前面的数据发送完成
    ACC = dat;                  //获取校验位P (PSW.0)
    busy = 1;
    SBUF = ACC;                 //写数据到UART数据寄存器
}

/*----------------------------
发送字符串
----------------------------*/
void SendString(char *s)
{
    while (*s)                  //检测字符串结束标志
    {
        SendData(*s++);         //发送当前字符
    }
}



/** COM receive message interrupt function**/

void Uart(void) interrupt 4 using 1
{
    if (RI)
    {
        RI = 0;                 //清除RI位
        //P0 = SBUF;              //P0显示串口数据
        //P22 = RB8;              //P2.2显示校验位
    }
    if (TI)
    {
        TI = 0;                 //清除TI位
        busy = 0;               //清忙标志
    }

}


