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

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/*******************************************************************************
 * MODEM UART instance and clock definitions
 ******************************************************************************/
#define MODEM_UART            UART3
#define MODEM_UART_CLKSRC     UART3_CLK_SRC
#define MODEM_UART_CLK_FREQ   CLOCK_GetFreq(MODEM_UART_CLKSRC)
#define MODEM_UART_RX_TX_IRQn UART3_RX_TX_IRQn
#define MODEM_UART_BAUDRATE	  115200

/*******************************************************************************
 * Buffer Definitions
 ******************************************************************************/
#define MODEM_RX_BUFFER_LEN 		255
#define MODEM_RX_LISTEN_ATTEMPTS	255
#define MAX_DEGREES					100

/*******************************************************************************
 * HW Definitions AND PHONE NUMBER
 ******************************************************************************/
//#define PHONE_NUMBER	"\"3332304490\""
//#define PHONE_NUMBER	"\"3320690296\""
#define PHONE_NUMBER	"\"3318609739\""
//#define PHONE_NUMBER	"\"3336676652\""
#define MODEM_HW_ID		"MODEM HWID: MP06212629FDA1"
#define GENERATOR_ID	"GENERATOR ID: DORIAN-1"

/*******************************************************************************
 * Generator error list
 ******************************************************************************/
const char WARNING_LOW_BATTERY[]	= 	"WARNING: LOW BATTERY DETECTED, PLEASE CHARGE";
const char WARNING_OVERHEATING[]	=	"ERROR: GENERATOR OVERHEARING";
const char SWARM_RX_FAILURE[]		=	"ERROR: SWARM DEVICE RX FAIL";
const char UNKOWN_ERROR[]			=	"UNKNOWN ERROR ON GENERATOR";
const char GENERATOR_RX_FAILURE[]	=	"ERROR: GENERATOR RX FAIL";
const char LOCATION_TEST[]			=	"LOCATION: 20.755726, -103.439900";
/*******************************************************************************
 * Buffer Variables
 ******************************************************************************/
char modem_rx_buffer[MODEM_RX_BUFFER_LEN] 	= 	{0};
char gps_coordinates[MODEM_RX_BUFFER_LEN] 	= 	{0};
char latitude_degree_dd[MAX_DEGREES] 	  	=	{0};
char longitude_degree_dd[MAX_DEGREES] 		= 	{0};

/*******************************************************************************
 * MODEM COMMON RESPONSES
 ******************************************************************************/
const char R_OK[]    = "OK";
const char R_ERROR[] = "ERROR";
const char R_READY[] = "READY";
const char R_BOOT[]  = "PB DONE";

/*******************************************************************************
 * MODEM AT SIM7600CE COMMANDS
 ******************************************************************************/
const char MODEM_ECHO_DISABLED[] 	= 	"ATE0";		//Disable Echo command from modem
const char MODEM_ECHO_ENABLED[]		=	"ATE";		//Enables Echo command from modem
const char MODEM_TEST[] 			=   "AT";			//Test UART communication
const char MODEM_RESET[]			=   "AT+CRESET";  //Reset the A7 modem
const char MODEM_VERSION[]  		=	"AT+GMR";     //View A7 modem info
const char MODEM_SIM_STATUS[] 		=	"AT+CPIN";	//SIM state
const char MODEM_GET_PLMN[]			= 	"AT+COPS";    //PLMN name and access network technology
const char MODEM_STATE[] 			= 	"AT+CFUN";    //Current modem operating mode state
const char MODEM_SMS_TEXT_FORMAT[] 	= 	"AT+CMGF";
const char MODEM_SEND_SMS[]			= 	"AT+CMGS";
const char MODEM_GPS_AUTO[]			=	"AT+CGPSAUTO";
const char MODEM_GPS_INFO[]			=	"AT+CGPSINFO";
const char EXTERNAL_PHONE_NUMBER[]	=	PHONE_NUMBER;
const char MODEM_SET_ON[] 			= 	"1";
const char MODEM_SET_OFF[] 			= 	"0";
const char MODEM_PARAM_EMPTY[] 		= 	""; 				//Used only for queries

/*
 * MODEM Command type
 *
 * brief: typedef enum for all the AT commands available.
 *
 * */
typedef enum modem_command_type {
	MODEM_CMD_QUERY,
	MODEM_CMD_SET,
	MODEM_CMD_EXECUTE
}modem_command_type;

/*
 * AT Commands structure to be reused and avoid to use more variables.
 *
 * param: command - AT command to execute
 * param: parameter - AT command parameter to be set
 * param: padded_len - Save the length of the parameter
 *
 *
 * */
typedef struct command_t{
	uint8_t* command;
	uint8_t* parameter;
	uint8_t* generator_error;
	size_t padded_len;
}command_t;



 #endif


