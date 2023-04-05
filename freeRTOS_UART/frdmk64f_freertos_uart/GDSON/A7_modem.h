/*
 * A7_modem.h
 * GDSON Project - MEXICO
 * Created on: 2 abr 2023
 *      Author: Dorian Martinez and Josue Martinez
 */
#ifndef __A7_MODEM_H__
#define __A7_MODEM_H__

#include "fsl_uart_freertos.h"
#include "fsl_uart.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "clock_config.h"
#include "pin_mux.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* MODEM UART instance and clock */
#define MODEM_UART            UART3
#define MODEM_UART_CLKSRC     UART3_CLK_SRC
#define MODEM_UART_CLK_FREQ   CLOCK_GetFreq(MODEM_UART_CLKSRC)
#define MODEM_UART_RX_TX_IRQn UART3_RX_TX_IRQn
#define MODEM_UART_BAUDRATE	  115200

/*******************************************************************************
 * Buffer Definitions
 ******************************************************************************/
#define MODEM_RX_BUFFER_LEN 512

/*******************************************************************************
 * MODEM COMMON RESPONSES
 ******************************************************************************/
const char R_OK[]    = "OK\r\n";
const char R_ERROR[] = "ERROR\r\n";
const char R_READY[] = "READY";

/*******************************************************************************
 * MODEM AT SIM7600CE COMMANDS
 ******************************************************************************/
const char MODEM_TEST[] 		=   "AT";			//Test UART communication
const char MODEM_RESET[]		=   "AT+CRESET";  //Reset the A7 modem
const char MODEM_VERSION[]  	=	"AT+GMR";     //View A7 modem info
const char MODEM_SIM_STATUS[] 	=	"AT+CPIN";	//SIM state
const char MODEM_GET_PLMN[]		= 	"AT+COPS";    //PLMN name and access network technology
const char MODEM_OPERATING_MODE[] = "AT+CFUN";    //Current modem operating mode state
const char MODEM_SMS_TEXT_FORMAT[] = "AT+CMGF";
const char MODEM_SET_ON[] = "1";
const char MODEM_SET_OFF[] = "0";
const char MODEM_PARAM_EMPTY[] = ""; //Used only for queries
/*MODEM Command type*/
typedef enum modem_command_type {
	MODEM_CMD_QUERY,
	MODEM_CMD_SET,
	MODEM_CMD_SEND_SMS
}modem_command_type;

typedef struct command_t{
	uint8_t* command;
	uint8_t* parameter;
	size_t padded_len;
}command_t;

/*******************************************************************************
 * UART MODEM SETTINGS
 ******************************************************************************/
uart_rtos_handle_t modem_handle;
struct _uart_handle t_modem_handle;
char background_buffer[MODEM_RX_BUFFER_LEN];
unsigned int bufferHead;

uart_rtos_config_t uart_modem_config = {
    .baudrate    = MODEM_UART_BAUDRATE,
    .parity      = kUART_ParityDisabled,
    .stopbits    = kUART_OneStopBit,
    .buffer      = background_buffer,
    .buffer_size = sizeof(background_buffer),
};


#endif


