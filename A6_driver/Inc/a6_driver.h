#ifndef __A6_DRIVER_H__
#define __A6_DRIVER_H__

#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx.h"

/*Prototypes*/
void debug_uart_init(void);
static uint16_t compute_uart_bd(uint32_t periph_clk,uint32_t baudrate);
static void debug_uart_write(int ch);
void a6_uart_init(void);
void a6_uart_write_char(char ch);
void systick_delay_ms(uint32_t delay);
/*******************************************/

#endif
