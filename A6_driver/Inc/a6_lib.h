#ifndef __A6_LIB_H__
#define __A6_LIB_H__
#include "a6_driver.h"
#include "fifo.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*Prototypes*/
void a6_module_init(void);
uint8_t a6_startup_test(void);
static void cpy_to_uart(void);
static void uart_output_char(char data);
static char to_lower(char letter);
static void wait_resp(char *pt);
static void search_check(char letter);
static void a6_process_data(void);
static void a6_send_cmd(const char * cmd);
void USART1_IRQHandler(void);
uint8_t a6_send_sms(char* message, char* phone_number);
uint8_t a6_dial_call(char* phone_number);
/*******************************************/

#endif
