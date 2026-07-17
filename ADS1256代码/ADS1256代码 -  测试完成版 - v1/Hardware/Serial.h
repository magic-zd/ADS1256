#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"

void USART1_Init(u32 bound);
void USART1_SendData(u8 data);
void USART1_SendString(char *str);

#endif
