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

   ADS1256_1_Init();
   ADS1256_2_Init();


    while(1)
    {
        // ============= 读第一片 =============
			        for(i=1; i<=4; i++)
        {
            adc  = ADS1256_2_ReadChannel(i);
            volt = ADS1256_ToVoltage(adc);
            printf("CH%d: %.4f V\r\n", i, volt);

        }
				
        // ============= 读第二片 =============
        for(i=1; i<=4; i++)
        {
            adc  = ADS1256_1_ReadChannel(i);
            volt = ADS1256_ToVoltage(adc);
            printf("CH%d: %.4f V\r\n", i+4, volt);
           
        }
        Delay_ms(500);

    }
}
