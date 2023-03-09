#include "a6_lib.h"


/*Defines*/
#define SR_TXE	(1U<<7)
#define SR_RXNE (1U<<5)
#define MAX_NUM_OF_TRY		  10
/*******************************/

/*Variables*/
char sub_str[32];
volatile bool searching		= false;
volatile bool is_response	= false;
volatile int search_idx		= 0;
/*******************************/

/*Prototypes*/
void a6_module_init(void);
uint8_t a6_get_sim_state(void);
static void cpy_to_uart(void);
static void uart_output_char(char data);
static char to_lower(char letter);
static void wait_resp(char *pt);
static void search_check(char letter);
static void a6_process_data(void);
static void a6_send_cmd(const char * cmd);
void USART1_IRQHandler(void);
uint8_t a6_get_signal_quality(void);
uint8_t a6_get_network_registration(void);
uint8_t a6_get_module_name(void);
/*******************************/

void a6_module_init(void)
{
	/*Enable FIFO*/
	tx_fifo_init();
	rx_fifo_init();

	/*Enable A6 UART*/
	a6_uart_init();

	/*Enable debug UART*/
	debug_uart_init();

	/*Initialize Flags*/
	searching		= false;
	is_response		= false;

	printf("A6 Initzialization...\n\r");

	/*Enable UART IRQ*/
	NVIC_EnableIRQ(USART1_IRQn);

	/*Get Module Name*/
	a6_get_module_name();

	/*Run test command*/
	a6_get_sim_state();

	/*Network registration*/
	a6_get_network_registration();

	/*Check Signal Quality*/
	a6_get_signal_quality();
}

uint8_t a6_dial_call(char* phone_number)
{
	printf("DEBUG_INFO: a6_dial_call function called\r\n");
	uint8_t num_of_try = MAX_NUM_OF_TRY;
	wait_resp("ok\r\n");
	while(num_of_try)
	{
		/*Phone number variable*/
		char AT_CMD[16] = "ATD+";
		char CR[] = "\r\n";
		strcat(AT_CMD, phone_number);
		strcat(AT_CMD, CR);
		a6_send_cmd(AT_CMD);
		if(is_response){
			printf("DEBUG_INFO: Call in progress...\r\n");
			return 1;
		}
		num_of_try--;
	}
	printf("DEBUG_INFO: Call failed...\r\n");
	return 0;

}

/*
 * Send an SMS
 * This function has two pointer variables to point to the message and phone number
 * The SMS will be sent on text format
 * */

uint8_t a6_send_sms(char* message, char* phone_number)
{

	printf("DEBUG_INFO: a6_send_sms function called\r\n");
	uint8_t num_of_try = MAX_NUM_OF_TRY;
	wait_resp("ok\r\n");

	while(num_of_try)
	{
		char SMS_AT_CMD1[] = "AT+CMGF=1\r\n";
		char SMS_AT_CMD2[21] = "AT+CMGS="; /*Country code*/
		char CR[] = "\r\n";

		uint8_t MSG_END[] = {26};

		/*Combine strings into single 1*/
		strcat(SMS_AT_CMD2,phone_number);
		strcat(SMS_AT_CMD2,CR);
		/*Set message format to text*/
		a6_send_cmd(SMS_AT_CMD1);
		systick_delay_ms(750);
		/*Send Phone Number*/
		a6_send_cmd(SMS_AT_CMD2);
		systick_delay_ms(750);

		/*Send the message*/
		a6_send_cmd(message);
		systick_delay_ms(750);
		a6_send_cmd((char *) MSG_END);
		systick_delay_ms(5000);
		if(is_response){
			return 1;
		}
		num_of_try--;

	}
	printf("DEBUG_INFO: SMS send failure\r\n");
	return 0;

}

/*Get Module Name*/
uint8_t a6_get_module_name(void)
{

	printf("DEBUG_INFO: a6_get_module_name function called\r\n");
	//ATI
	uint8_t num_of_try = MAX_NUM_OF_TRY;
	/*This initialize the value that we are waiting for...*/
	wait_resp("ok\r\n");
	while(num_of_try)
	{
		a6_send_cmd("ATI\r\n");
		/*Wait 3 seconds*/
		systick_delay_ms(3000);
		if(is_response)
		{
			/*Success*/
			printf("DEBUG_INFO: a6_get_module_name function success\r\n");
			return 1;
		}
		num_of_try--;
	}
	printf("DEBUG_INFO:Err: a6_get_module_name function failed\r\n");
	return 0;
}

/*Check SIM state*/
uint8_t a6_get_sim_state(void)
{
	printf("DEBUG_INFO: a6_get_sim_state function called\r\n");
	//AT+CPIN?
	uint8_t num_of_try = MAX_NUM_OF_TRY;
	/*This initialize the value that we are waiting for...*/
	wait_resp("ok\r\n");
	while(num_of_try)
	{
		a6_send_cmd("AT+CPIN?\r\n");
		/*Wait 3 seconds*/
		systick_delay_ms(3000);
		if(is_response)
		{
			/*Success*/
			printf("DEBUG_INFO: a6_get_sim_state function success\r\n");
			return 1;
		}
		num_of_try--;
	}
	printf("DEBUG_INFO:Err: a6_get_sim_state failure\r\n");
	return 0;
}

/*Check Network Registration*/
uint8_t a6_get_network_registration(void)
{
	printf("DEBUG_INFO: a6_get_network_registration function called\r\n");
	//AT+COPS?
	uint8_t num_of_try = MAX_NUM_OF_TRY;
	/*This initialize the value that we are waiting for...*/
	wait_resp("ok\r\n");
	while(num_of_try)
	{
		a6_send_cmd("AT+CREG=?\r\n");
		/*Wait 3 seconds*/
		systick_delay_ms(3000);
		if(is_response)
		{
			/*Success*/
			printf("DEBUG_INFO: a6_get_network_registration success\r\n");
			return 1;
		}
		num_of_try--;
	}
	printf("DEBUG_INFO:Err: a6_get_network_registration failure\r\n");
	return 0;
}


/*Signal Quality Function*/
uint8_t a6_get_signal_quality(void)
{
	printf("DEBUG_INFO: a6_get_signal_quality function called\r\n");
	//AT+CSQ=?
	uint8_t num_of_try = MAX_NUM_OF_TRY;
	/*This initialize the value that we are waiting for...*/
	wait_resp("ok\r\n");
	while(num_of_try)
	{
		a6_send_cmd("AT+CSQ=?\r\n");
		/*Wait 3 seconds*/
		systick_delay_ms(3000);
		if(is_response)
		{
			/*Success*/
			printf("DEBUG_INFO: a6_get_signal_quality success\r\n");
			return 1;
		}
		num_of_try--;
	}
	printf("DEBUG_INFO:Err: a6_get_signal_quality failure\r\n");
	return 0;
}


/*Copy content of tx_fifo into debug UART*/
static void cpy_to_uart(void)
{
	char letter;
	/*Make sure transmit data register is empty and fifo has content*/
	/*BIT 7 is for TXE: Transmit data register is empty*/
	while((USART2->SR  & SR_TXE) && tx_fifo_size() >0)
	{
		/*Get char from fifo*/
		tx_fifo_get(&letter);

		/*Put into uart data register*/
		USART2->DR = letter;
	}

}


/*Output UART Character*/
static void uart_output_char(char data)
{

	if(tx_fifo_put(data) == FIFOFAIL)
	{
		return;
	}

	cpy_to_uart();

}

static char to_lower(char letter)
{
	if((letter >= 'A') && (letter <= 'Z'))
	{
		letter |= 0x20;
	}

	return letter;
}

/*Initialize the string search in the RX data stream*/
/*When an AT command is sent  we need to wait for the response of the command*/

static void wait_resp(char *pt)
{
	/*The below code will copy the content of string1 to string2.*/
	strcpy(sub_str,pt);
	search_idx 	= 0;
	is_response = false;
	searching 	= true;

}

/*Search for string in rx data stream*/
static void search_check(char letter)
{
	if(searching)
	{
		/*Check if character match*/
		if(sub_str[search_idx] == to_lower(letter)){
			search_idx++;

			/*Check if end of string match*/
			if(sub_str[search_idx] == 0)
			{
				is_response = true;
				searching 	= false;

			}
		}
		else
		{
			/*Start over*/
			search_idx = 0;
		}
	}
}

static void a6_process_data(void){

	char letter;
	/*Check if there is new data in data register*/
	if(USART1->SR & SR_RXNE){

		/*Store data from A6 uart data register to local variable*/
		letter = USART1->DR;

		/*Print from A6 uart data register to debug UART, meaning your computer*/
		uart_output_char(letter);

		/*Check for response*/
		search_check(letter);

	}

}

/*Send command to A6*/
static void a6_send_cmd(const char * cmd)
{
	int index = 0;
	while(cmd[index] != 0)
	{
		a6_uart_write_char(cmd[index++]);
	}

}


void USART1_IRQHandler(void)
{
	a6_process_data();
}
