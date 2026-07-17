#include "stm32f10x.h"
#include "delay.h"
#include "usart.h"
#include "ads1256.h"
#include <stdio.h>

int main(void)
{
    int32_t adc;
    float volt;
    uint8_t i;

    Delay_Init();
    USART1_Init(9600);
    ADS1256_Init();

    printf("ADS1256 8通道稳定测试开始\r\n");

    while(1)
    {
        for(i = 1; i <= 4; i++)
        {

            adc = ADS1256_ReadChannel(i);
            volt = ADS1256_ToVoltage(adc);
						printf("CH%d: %.4f V\r\n", i,volt);
					Delay_ms(200);
//					printf("CH%d: %.4f V%u\r\n", i,volt,ADS1256_ReadID());
        }

//        printf("------------------------\r\n");
       
    }
}









