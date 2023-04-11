/*
 * Copyright (c) 2023, Dorian Martinez and Josue Martinez
 * GDSON Project - MEXICO
 * All rights reserved.
 *
 */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_uart_freertos.h"
#include "fsl_uart.h"
#include <stdbool.h>
#include "A7_modem.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define uart_modem_task_PRIORITY (configMAX_PRIORITIES - 1)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void modem_task(void *pvParameters);
void modem_uart_init();
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool success_command = false;
volatile bool attempts_until_failed = false;
volatile bool ok_received = false;
volatile bool error_received = false;
volatile bool ready_received = false;
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application entry point.
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();


    PRINTF("##############################################\r\n");
    PRINTF("GDSON PROTOTYPE v1.0\r\n");
    PRINTF("DORIAN MARTINEZ AND JOSUE MARTINEZ\r\n");
    PRINTF("MEXICO, JALISCO 20/03/2023\r\n");
    PRINTF("##############################################\r\n");

    if (xTaskCreate(modem_task, "Modem_task", (configMINIMAL_STACK_SIZE + 100)*2, NULL, uart_modem_task_PRIORITY, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
    vTaskStartScheduler();
    for (;;)
        ;
}

/*!
 * @brief Task responsible for loopback.
 */
static void modem_task(void *pvParameters)
{

    modem_uart_init();
    /*Clear existing buffer*/
    clear_modem_buffer();
    send_at_commands(MODEM_ECHO_DISABLED, MODEM_CMD_EXECUTE, MODEM_PARAM_EMPTY);

    send_at_commands(MODEM_OPERATING_MODE, MODEM_CMD_QUERY, MODEM_PARAM_EMPTY);
    get_modem_response_uart();

    //clear_modem_buffer();
    /*Clear existing buffer*/
    send_at_commands(MODEM_GET_PLMN, MODEM_CMD_QUERY, MODEM_PARAM_EMPTY);
    get_modem_response_uart();
    //clear_modem_buffer();
    send_at_commands(MODEM_SIM_STATUS, MODEM_CMD_QUERY, MODEM_PARAM_EMPTY);
    get_modem_response_uart();
    //clear_modem_buffer();

    UART_RTOS_Deinit(&modem_handle);
	ok_received = false;
	error_received = false;
	ready_received = false;
    vTaskSuspend(NULL);

}

/*
 * Initialize UART modem to send AT commands.
 *
 */

void modem_uart_init(){

	PRINTF("DEBUG INFO: modem_uart_init: \r\n");
	NVIC_SetPriority(MODEM_UART_RX_TX_IRQn, 5);
    uart_modem_config.srcclk = MODEM_UART_CLK_FREQ;
    uart_modem_config.base   = MODEM_UART;
    if (kStatus_Success == UART_RTOS_Init(&modem_handle, &t_modem_handle, &uart_modem_config))
    {
    	PRINTF("DEBUG INFO: modem_uart_init success\r\n");
    }else{
    	PRINTF("DEBUG INFO: modem_uart_init internal failure\r\n");
    	vTaskSuspend(NULL);
    }

}

/*
 * Function to use AT commands to query or set.
 */

void send_at_commands(const char * cmd, char type, const char * params)
{

	PRINTF("DEBUG INFO: send_at_commands: \r\n");

	command_t at_command;
    size_t cmd_len = strlen(cmd);
    size_t params_len = strlen(params);
    char *send_enter_key = "\r\n";

    /*Assign memory to command and parameter*/
    at_command.command = (uint8_t*) malloc(cmd_len + 1);
    at_command.parameter = (uint8_t*) malloc(params_len + 1);

    /*Copy the characters string from cmd and params to the elements of the structure*/
    memcpy(at_command.command, cmd, cmd_len + 1);
    memcpy(at_command.parameter, params, params_len + 1);

    /*Print the values of the command and parameters for debugging*/
    PRINTF("DEBUG INFO: Request Command: %s\n\r", at_command.command);
    PRINTF("DEBUG INFO: Parameter: %s\n\r", at_command.parameter);

    /*Assign memory for full command to combine AT command and parameters*/
    char* at_full_command = (char*) malloc(cmd_len + params_len + 2);

    if(type == MODEM_CMD_SET){
    	PRINTF("DEBUG INFO: MODEM_CMD_SET: \r\n");
        sprintf(at_full_command, "%s=%s", at_command.command, at_command.parameter);
        at_command.padded_len = strlen(at_full_command);
        PRINTF("DEBUG INFO: AT full command: %s\n\r", at_full_command);
    }
    if(type == MODEM_CMD_QUERY){
    	PRINTF("DEBUG INFO: MODEM_CMD_QUERY: \r\n");
        sprintf(at_full_command, "%s%s?", at_command.command, at_command.parameter);
        at_command.padded_len = strlen(at_full_command);
        PRINTF("DEBUG INFO: AT full command: %s\n\r", at_full_command);
    }
    if(type == MODEM_CMD_SEND_SMS)
    {
    	PRINTF("DEBUG INFO: MODEM_CMD_SEND_SMS: \r\n");
    	PRINTF("DEBUG INFO: WORK IN PROGRES - NOT IMPLEMENTED\r\n");
    }
    if(type == MODEM_CMD_EXECUTE)
    {
    	PRINTF("DEBUG INFO: MODEM_CMD_EXECUTE: \r\n");
        sprintf(at_full_command, "%s%s", at_command.command, at_command.parameter);
        at_command.padded_len = strlen(at_full_command);
        PRINTF("DEBUG INFO: AT full command: %s\n\r", at_full_command);
    }

	if(kStatus_Success == UART_RTOS_Send(&modem_handle, (uint8_t *)at_full_command, at_command.padded_len))
	{
		UART_RTOS_Send(&modem_handle, (uint8_t *)send_enter_key, strlen(send_enter_key));
		readByteToBuffer();
		PRINTF("DEBUG INFO: %s send success\r\n",at_full_command);


	}


    /*Release memory for all variables*/
    free(at_command.command);
    free(at_command.parameter);
    free(at_full_command);

}

void readByteToBuffer(void){

	size_t tries = 0;
	uint8_t incoming_data;
	int result;
	size_t len;

	do{

		UART_RTOS_Receive(&modem_handle, &incoming_data, 1, NULL);

	    tries++;
	    len++;
	    /*Save the incoming data into the buffer array*/
	    if(incoming_data != '\0'){

		    background_buffer[bufferHead] = incoming_data;
		    /*if we overflow then start at the begining of the buffer*/
		    bufferHead = (bufferHead + 1) % MODEM_RX_BUFFER_LEN;

		    result = check_modem_result(R_OK);
		    if(result != NULL){
		    	PRINTF("DEBUG INFO: MODEM RESULT: OK \r\n");
		    	ok_received = true;
		    }

		    result = check_modem_result(R_ERROR);
		    if(result != NULL){
		    	PRINTF("DEBUG INFO: MODEM RESULT: ERROR \r\n");
		    	error_received = true;
		    }

		    result = check_modem_result(R_READY);
		    if(result != NULL){
		    	PRINTF("DEBUG INFO: MODEM RESULT: READY \r\n");
		    	ready_received = true;
		    }
	    }

	    if(tries > MODEM_RX_LISTEN_ATTEMPTS){
	    	attempts_until_failed = true;
	    	PRINTF("DEBUG INFO: Max number of listen attempts reached: UART RX FAILED\r\n");
	    }

		}while(!attempts_until_failed && !ok_received && !error_received && !ready_received);


}


/*Clear buffer function*/
void clear_modem_buffer(void)
{
	//PRINTF("DEBUG INFO: clearBuffer set\r\n");
	memset(background_buffer, '\0', MODEM_RX_BUFFER_LEN);
	//PRINTF("DEBUG INFO: Buffer ready to listen\r\n");
}

int check_modem_result(const char * modem_resp)
{
	int result;
	int bufferLen = strlen((const char *)background_buffer);
	// If our buffer isn't full, just do an strstr
	if (bufferLen <= MODEM_RX_BUFFER_LEN)
	{
		/*Check the modem responses*/
		result = strstr((const char *)background_buffer, modem_resp);
		if (result != NULL)
		{
			return result;
		}

		return result;
	}

}

void get_modem_response_uart(void)
{

	PRINTF("DEBUG INFO: get_modem_response\r\n");
	int i;
	int bufferLen = strlen((const char *)background_buffer);
	for(i=0 ; i<bufferLen; i++)
	{
		PRINTF("%c", background_buffer[i]);
	}
	PRINTF("\r\n");


}
