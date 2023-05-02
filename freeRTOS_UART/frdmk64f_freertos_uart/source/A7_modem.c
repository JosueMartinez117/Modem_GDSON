/**!
 * Copyright (c) 2023, Dorian Martinez and Josue Martinez
 * GDSON Project - MEXICO
 * All rights reserved.
 *
 */

 /**! A7_modem.c
 * GDSON Project - MEXICO
 * Created on: 2 abr 2023
 *      Author: Dorian Martinez and Josue Martinez
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

#include "A7_modem.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*configMAX_PRIORITIES: 5*/
#define uart_modem_task_PRIORITY (configMAX_PRIORITIES - 1) //Means that priority for modem task is 4.
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
volatile bool modem_boot	 = false;
/*******************************************************************************
 * Code
 ******************************************************************************/
/**!
 * @brief Application entry point.
 */
int main(void)
{
    /* Init board hardware. */
	BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();


    PRINTF("##############################################\r\n");
    PRINTF("GDSON PROTOTYPE v5.1\r\n");
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

/**!
 * @brief
 *
 * modem_uart_init:
 *
 * Initialize UART to establish communication
 * with the modem to send and receive messages.
 *
 */

void modem_uart_init(){

	PRINTF("DEBUG INFO: modem_uart_init: \r\n");

	uart_config_t config;

	/*Gets the default UART configuration*/
	UART_GetDefaultConfig(&config);
	/*Enables TX and RX to send and get data*/
	config.enableTx = true;
	config.enableRx = true;

	/*UART Init returns 0 if the initialization was OK, if it is success the task will continue
	 * but if it is not success the task will be suspended and an error message will be displayed*/
    if (kStatus_Success == UART_Init(MODEM_UART, &config, MODEM_UART_CLK_FREQ))
    {
    	PRINTF("DEBUG INFO: modem_uart_init success\r\n");
    }else{
    	PRINTF("DEBUG INFO: modem_uart_init internal failure\r\n");
    	vTaskSuspend(NULL);
    }

}

/**!
 * @brief
 *
 * check_modem_result:
 *
 * param modem_resp receive the modem result from the function readByteToBuffer.
 *
 * Check the modem result to be analyzed in the code
 *
 */
int check_modem_result(const char * modem_resp)
{
	int result;
	/*Get the size of the buffer*/
	int bufferLen = strlen((const char *)modem_rx_buffer);

	if (bufferLen <= MODEM_RX_BUFFER_LEN)
	{
		/*Save the result of the function strstr if the result is not NULL means that the string requested was found*/
		result = strstr((const char *)modem_rx_buffer, modem_resp);
		if (result != NULL)
		{
			return result;
		}
		return result;
	}

}

/**!
 * @brief
 *
 * readByteToBuffer:
 *
 * Reads the incoming information from the modem byte to byte
 * and save all the information inside of the buffer modem_rx_buffer.
 *
 * The function compares the incoming data with the common modem responses:
 * OK, READY, ERROR, BOOT.
 *
 */
void readByteToBuffer(void){

	int tries = 0;
	uint8_t incoming_data;
	int result;
	size_t len = 0;

	do{

		/*Receive information byte to byte*/
		UART_ReadBlocking(MODEM_UART, &incoming_data, 1);

	    /*Save the incoming data into the buffer array*/
	    if(incoming_data != '\0'){

	    	modem_rx_buffer[len] = incoming_data;

	    	result = check_modem_result(R_OK);
	    	if(result != NULL){
	    		ok_received = true;
	    	}
	    	result = check_modem_result(R_ERROR);
			if(result != NULL){
	    		error_received = true;
	    	}
			result = check_modem_result(R_READY);
			if(result != NULL){
	    		ready_received == true;
	    	}
			result = check_modem_result(R_BOOT);
			if(result != NULL){
				modem_boot == true;
			}
	    }
	    if(tries > MODEM_RX_LISTEN_ATTEMPTS)
	    {
	    	attempts_until_failed = true;
	    }

	    len++;
	    tries++;
		}while(!attempts_until_failed && !ok_received && !error_received && !ready_received && !modem_boot);

	/*Reset all variables to be used in a next iteration*/
	ok_received = false;
	error_received = false;
	ready_received = false;
	modem_boot = false;
	tries = 0;

}

/**!
 * @brief
 *
 * send_at_commands:
 *
 * Function to use AT commands
 * param cmd receive the command to be used.
 * param type receive the type of the command, set, query or execute.
 * param parms receive the parameter to be used by the at command.
 *
 */

void send_at_commands(const char * cmd, char type, const char * params)
{

	PRINTF("DEBUG INFO: send_at_commands response: \r\n");

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
    if(type == MODEM_CMD_EXECUTE)
    {
    	PRINTF("DEBUG INFO: MODEM_CMD_EXECUTE: \r\n");
        sprintf(at_full_command, "%s%s", at_command.command, at_command.parameter);
        at_command.padded_len = strlen(at_full_command);
        PRINTF("DEBUG INFO: AT full command: %s\n\r", at_full_command);
    }

	if(kStatus_Success == UART_WriteBlocking(MODEM_UART, (uint8_t *)at_full_command, at_command.padded_len))
	{
		UART_WriteBlocking(MODEM_UART, (uint8_t *)send_enter_key, strlen(send_enter_key));
		readByteToBuffer();
		PRINTF("DEBUG INFO: %s send success\r\n",at_full_command);
	}


    /*Release memory for all variables*/
    free(at_command.command);
    free(at_command.parameter);
    free(at_full_command);

}

/**!
 * @brief
 *
 * send_at_command_sms:
 *
 * Function to use AT commands to send SMS.
 *
 * param cmd receive the command to be used.
 * param parms receive the parameter to be used by the at command.
 * param error_gen receive the incoming generator error.
 * param location receive the current coordinates or location of the generator.
 *
 */

void send_at_command_sms(const char * cmd, const char * params, const char * error_gen, const char * location)
{

	PRINTF("DEBUG INFO: send_at_command_sms response: \r\n");

	/*AT Commands to configure the SMS settings*/
	command_t at_command;

	/*Calculates size of incoming parameters*/
    size_t cmd_len = strlen(cmd);
    size_t params_len = strlen(params);
    size_t error_gen_len = strlen(error_gen);
    size_t location_len = strlen(location);

    char *send_enter_key = "\r\n";
    char *generator_location;
    char ctrl_z[] = { 0x1A, '\0' };

	/*Variables to save SMS content and length*/
	char* sms_body;
    size_t sms_body_len = strlen(MODEM_HW_ID) + strlen(GENERATOR_ID) + location_len + error_gen_len;

	/*Assign memory to command and parameter*/
	sms_body = (char*)malloc(sms_body_len * sizeof(char));

    /*Assign memory to command and parameter*/
    at_command.command = (uint8_t*) malloc(cmd_len + 1);
    at_command.parameter = (uint8_t*) malloc(params_len + 1);
    at_command.generator_error =  (uint8_t*) malloc(error_gen_len + 1);
    generator_location = (uint8_t*) malloc(location_len + 1);

    /*Copy the characters string from cmd and params to the elements of the structure*/
    memcpy(at_command.command, cmd, cmd_len + 1);
    memcpy(at_command.parameter, params, params_len + 1);
    memcpy(at_command.generator_error, error_gen, error_gen_len +1);
    memcpy(generator_location, location, location_len + 1);

    /*Print the values of the command and parameters for debugging*/
    PRINTF("DEBUG INFO: Request Command: %s\n\r", at_command.command);
    PRINTF("DEBUG INFO: Parameter: %s\n\r", at_command.parameter);
    PRINTF("DEBUG INFO: Generator error: %s\n\r", at_command.generator_error);
    PRINTF("DEBUG INFO: Generator location: %s\n\r", generator_location);

    /*Assign memory for full command to combine AT command and parameters*/
    char* at_full_command = (char*) malloc(cmd_len + params_len + 2);

    /*Set SMS Format*/
    PRINTF("DEBUG INFO: MODEM_CMD_SEND_SMS: \r\n");
    sprintf(at_full_command, "%s=%s", at_command.command, at_command.parameter);
    at_command.padded_len = strlen(at_full_command);
    sprintf(sms_body, "%s\n\r%s\n\r%s\r%s\n\r%s", MODEM_HW_ID, GENERATOR_ID, "LOCATION: ",generator_location, at_command.generator_error);
    PRINTF("DEBUG INFO: SMS BODY: %s \r\n", sms_body);

	if(kStatus_Success == UART_WriteBlocking(MODEM_UART, (uint8_t *)at_full_command, at_command.padded_len))
	{
		UART_WriteBlocking(MODEM_UART, (uint8_t *)send_enter_key, strlen(send_enter_key));
		vTaskDelay(pdMS_TO_TICKS(1000));
		UART_WriteBlocking(MODEM_UART, (uint8_t *)sms_body, strlen(sms_body));
		UART_WriteBlocking(MODEM_UART, (uint8_t *)ctrl_z, strlen(ctrl_z));
		readByteToBuffer();
		PRINTF("DEBUG INFO: %s send success\r\n",at_full_command);

	}


    /*Release memory for all variables*/
    free(at_command.command);
    free(at_command.parameter);
    free(at_full_command);
    free(at_command.generator_error);
    free(generator_location);
}



/**!
 *
 * @brief
 *
 * clear_modem_buffer:
 *
 * The function clean all the data saved in the modem buffer: modem_rx_buffer
 * the function use memset to fill all the information of the buffer with '\0'
 *
 */
void clear_modem_buffer(void)
{
	memset(modem_rx_buffer, '\0', MODEM_RX_BUFFER_LEN);
}

/**!
 * @brief
 *
 * get_modem_response_uart:
 *
 * The function displays all the information received by the modem into the UART0
 * to debug the current state of the modem.
 *
 */
void get_modem_response_uart(void)
{

	PRINTF("DEBUG INFO: get_modem_response_uart response\r\n");
	int i;
	int bufferLen = strlen((const char *)modem_rx_buffer);
	for(i=0 ; i<bufferLen; i++)
	{
		PRINTF("%c", modem_rx_buffer[i]);
	}
	PRINTF("\r\n");
}

/**!
 * @brief
 *
 * check_modem_state:
 *
 * Guarantees the SIM state is ready to be used and set the modem to Online state.
 * In case the SIM card is not available / inserted, the task will be suspended.
 * If a SIM card is not inserted the modem will be useless.
 *
 */
BaseType_t check_modem_state(void)
{

	PRINTF("DEBUG INFO: check_modem_state response\r\n");

	int result;

	/*Send the AT+CPIN? command to query SIM state*/
    send_at_commands(MODEM_SIM_STATUS, MODEM_CMD_QUERY, MODEM_PARAM_EMPTY);
    result = check_modem_result(R_READY);
    get_modem_response_uart();

    /*If the result is READY display the modem response*/
    if(result != NULL){
    	get_modem_response_uart();
    	PRINTF("DEBUG INFO: SIM READY: MODEM READY TO BE USED\r\n");
    	return pdPASS;
    }else{
    	/*If the result is not ready set the modem to online mode*/
    	clear_modem_buffer();
    	send_at_commands(MODEM_STATE, MODEM_CMD_SET, MODEM_SET_ON);
    	vTaskDelay(pdMS_TO_TICKS(1200));
    	get_modem_response_uart();
    	result = check_modem_result(R_OK);

    	/*If the result is OK check again the status of the SIM card*/
    	if(result != NULL){

    		clear_modem_buffer();
    		send_at_commands(MODEM_SIM_STATUS, MODEM_CMD_QUERY, MODEM_PARAM_EMPTY);
    		result = check_modem_result(R_READY);
    		get_modem_response_uart();

    		/*If the result is READY the SIM card is inserted but the modem was off in the first attempt*/
    		if(result != NULL){
    	    	PRINTF("DEBUG INFO: SIM READY: MODEM READY TO BE USED\r\n");
    	    	return pdPASS;
    		}else{

        		/*If the result is not ready the SIM card is not inserted and the modem will be useless*/
    	    	PRINTF("DEBUG INFO: SIM NOT INSERTED: FATAL ERROR\r\n");
    	    	PRINTF("DEBUG INFO: WARNING: TASK WILL BE SUSPENDED\r\n");
    	    	PRINTF("DEBUG INFO: WARNING: IT IS NOT POSSIBLE TO USE THE MODEM\r\n");
    	    	return pdFAIL;
    		}
    	}else{
    		PRINTF("DEBUG INFO: ERROR: check_modem_state: MODEM_STATE_ON: fail\r\n");
    	}

    }

}

/**!
 * @brief
 *
 * get_network_state:
 *
 * This function get the network state of the modem and
 * guarantees that the device is registered to the network.
 *
 *
 */
BaseType_t get_network_state(void){

	PRINTF("DEBUG INFO: get_network_state response\r\n");

	int result;

	/*Send the command AT+COPS? to get the current network registration*/
    send_at_commands(MODEM_GET_PLMN, MODEM_CMD_QUERY, MODEM_PARAM_EMPTY);
    result = check_modem_result(R_OK);
    get_modem_response_uart();

    /*If result is OK the then the string will be compared*/
    if(result != NULL)
    {
    	/*Compare the data inside the buffer with the number seven, if the number
    	 * is found in the data means that the device is registered to LTE network*/

    	if(strstr((const char *)modem_rx_buffer, "7") != NULL)
    	{
    		PRINTF("DEBUG INFO:  DEVICE REGISTERED: NETWORK TYPE: LTE\r\n");
    		return pdPASS;
    	}else{
    		PRINTF("DEBUG INFO:  DEVICE IS NOT REGISTERED TO THE NETWORK\r\n");
    		return pdFAIL;
    	}

    }else{
    	PRINTF("DEBUG INFO: ERROR: get_network_state: MODEM_GET_PLMN: fail\r\n");
    }

}

/**!
 * @brief
 *
 * ddm_to_dd:
 *
 * Converts the coordinates in decimal degrees and minutes (DDM) to decimal degrees (DD)
 *
 * */

double ddm_to_dd(double ddm) {

    double degrees = floor(ddm / 100.0);
    double minutes = ddm - degrees * 100.0;
    double decimal_degrees = degrees + minutes / 60.0;
    return decimal_degrees;

}

/**!
 * @brief
 *
 * get_gps_info:
 *
 * This function gets the current location of the device, the coordinates
 * are reported by the modem in DMS format.
 *
 * */
BaseType_t get_gps_info(void){

	PRINTF("DEBUG INFO: get_gps_info response\r\n");

	int result;

	/*Variables to save the current coordinates and direction*/
	char latitude_dir, longitude_dir;
	double lat_deg , lon_deg;
	double decimal_latitude_float , decimal_longitude_float;

	/*Set the GPS in Auto*/
	send_at_commands(MODEM_GPS_AUTO, MODEM_CMD_SET, MODEM_SET_ON);
    result = check_modem_result(R_OK);
    get_modem_response_uart();

    /*Clean GPS coordinates for previous execution*/
    memset(gps_coordinates, '\0', MODEM_RX_BUFFER_LEN);

    if(result != NULL){
    	clear_modem_buffer();
    	vTaskDelay(pdMS_TO_TICKS(1200));

    	/*Get GPS Coordinates in DMS format*/
    	send_at_commands(MODEM_GPS_INFO, MODEM_CMD_EXECUTE, MODEM_PARAM_EMPTY);
		get_modem_response_uart();
		result = check_modem_result(R_OK);

		if(result != NULL){

			/*Save the coordinates to be sent*/
			char* pch = strstr(modem_rx_buffer, ":");
			if(pch != NULL){
				pch++;
				pch++;

				/*gps_coordinates -> Array to save the location*/
				strncpy(gps_coordinates, pch, strlen(pch));

				/*length of the array and remove the unused data*/
				size_t gps_coordinates_len = strlen(gps_coordinates);

				if(gps_coordinates_len >= 2 && strcmp(gps_coordinates + gps_coordinates_len - 2, "OK") == 0){
					gps_coordinates[gps_coordinates_len - 2] = '\0';
				}

				/*Search inside the gps_coordinates buffer to save the information of latitude , longitude and direction*/
				sscanf(gps_coordinates, "%lf,%c,%lf,%c", &lat_deg, &latitude_dir, &lon_deg, &longitude_dir);

				/*Save the coordinates in the format DD*/
				decimal_latitude_float = ddm_to_dd(lat_deg);
				decimal_longitude_float = ddm_to_dd(lon_deg);

				if (latitude_dir == 'S') {
					decimal_latitude_float = -decimal_latitude_float;
				}
				if (longitude_dir == 'W') {
					decimal_longitude_float = -decimal_longitude_float;
				}


				/*Clean GPS buffers*/
				memset(gps_coordinates, '\0', MODEM_RX_BUFFER_LEN);
				sprintf(gps_coordinates, "https://maps.google.com/maps?f=q&q=%lf,%lf&z=16", decimal_latitude_float , decimal_longitude_float);

			}

			PRINTF("DEBUG INFO: get_gps_info: success \r\n");
			return pdPASS;
		}

    }else{
    	PRINTF("DEBUG INFO: ERROR: get_gps_info: MODEM_GPS_AUTO fail\r\n");
    	return pdFAIL;
    }

}


/**!
 * @brief
 *
 * network_scan:
 *
 * This function performs a strategy to get registered to the Network.
 *
 */
BaseType_t network_scan(void)
{


	PRINTF("DEBUG INFO: network_scan response\r\n");

	int result;

	/*Send the command AT+CFUN=0 to move the modem to airplane state*/
	send_at_commands(MODEM_STATE, MODEM_CMD_SET, MODEM_SET_OFF);
	get_modem_response_uart();
	result = check_modem_result(R_OK);

	/*If result is OK turn off the modem*/
	if(result != NULL){
		clear_modem_buffer();
		send_at_commands(MODEM_STATE, MODEM_CMD_SET, MODEM_SET_ON);
		get_modem_response_uart();
		result = check_modem_result(R_OK);
		if(result != NULL){
			vTaskDelay(pdMS_TO_TICKS(10000));
			PRINTF("DEBUG INFO: network_scan completed\r\n");
			return pdPASS;
		}
	}else{
		PRINTF("DEBUG INFO: ERROR: network_scan call function fail\r\n");
		return pdFAIL;
	}

}

/*!
 * @brief Task responsible for loopback.
 */
static void modem_task(void *pvParameters)
{
	BaseType_t rtos_result;
	int network_scan_retries = 0;
	volatile bool network_scan_flag = false;

	/*Initialize MODEM UART communication*/
    modem_uart_init();

    /*Clear buffer to release memory for previous executions*/
    clear_modem_buffer();

    /*Verify if the modem is ready to be used*/
    rtos_result = check_modem_state();
    if(rtos_result == pdPASS){
    	PRINTF("DEBUG INFO: check_modem_state function call success\r\n");

        /*Clear buffer to release memory for previous executions*/
        clear_modem_buffer();

        /*Get GPS info*/
        get_gps_info();

        /*Get network state
         *
         * If device is registered to the network a SMS will be sent
         * */

        clear_modem_buffer();

        rtos_result = get_network_state();
        if(rtos_result == pdPASS){

        	clear_modem_buffer();
        	/*Send SMS */
       	    send_at_command_sms(MODEM_SEND_SMS, EXTERNAL_PHONE_NUMBER, SWARM_RX_FAILURE, gps_coordinates);
       	    rtos_result = check_modem_result(R_OK);
        	if(rtos_result != NULL)
        	{
        	    PRINTF("DEBUG INFO: SEND SMS success\r\n");
        	}else{
        	    PRINTF("DEBUG INFO: SEND SMS fail\r\n");
        	    }

        }else
        { /*If device is not registered registered to the network do a network recovery strategy
         	 	 the SMS will be sent once the device is registered to the network*/

        	do{

        		/*Clear buffer to release memory for previous executions*/
				clear_modem_buffer();
				/*Perform a network scan*/
            	network_scan();

        		/*Clear buffer to release memory for previous executions*/
				clear_modem_buffer();
            	/*Check the network state*/
            	rtos_result = get_network_state();

            	if(rtos_result == pdPASS){

            		network_scan_flag = true;
            		/*Due to the device is registered the SMS will be sent*/
               	    send_at_command_sms(MODEM_SEND_SMS, EXTERNAL_PHONE_NUMBER, SWARM_RX_FAILURE, LOCATION_TEST);
               	    rtos_result = check_modem_result(R_OK);
                	if(rtos_result != NULL){
                	    	PRINTF("DEBUG INFO: SEND SMS success\r\n");
                	}else{
                	    	PRINTF("DEBUG INFO: SEND SMS fail\r\n");
                	   	 }

            	}

            	/*if the network was not registered in five attempts then the network registration fails*/
        		if(network_scan_retries > 5){
        			network_scan_flag = true;
        			PRINTF("DEBUG INFO: ERROR: network registration fail\r\n");
        		}

        	network_scan_retries++;
        	}while(network_scan_flag == false);


        }

    }else{ /*If Modem is not ready the task will be suspended*/
    	PRINTF("DEBUG INFO: check_modem_state function call fail\r\n");
    	vTaskSuspend(NULL);
    }


    PRINTF("DEBUG INFO: modem_task completed!!\r\n");

	/*Clear buffer to release memory for previous executions*/
	clear_modem_buffer();

    /*Disable MODEM UART communication*/
    UART_Deinit(MODEM_UART);
    vTaskSuspend(NULL);

}


