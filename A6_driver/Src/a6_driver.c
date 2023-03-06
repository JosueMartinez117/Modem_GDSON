#include "a6_driver.h"

/*Defines*/
#define GPIOAEN			(1U<<0) // 0B 0000 0000 0000 0000 0000 0000 0000 0001 /*RCC AHB1 RCC_AHB1ENR - Peripheral clock enable register*/
#define UART2EN			(1U<<17)
#define CR1_UE			(1U<<13)
#define CR1_RE			(1U<<2)
#define CR1_TE			(1U<<3)
#define SR_TXE			(1U<<7)
#define UART_BAUDRATE	115200
#define SYS_FREQ		16000000
/*******************************************/

/*Prototypes*/
void debug_uart_init(void);
static uint16_t compute_uart_bd(uint32_t periph_clk,uint32_t baudrate);
static void debug_uart_write(int ch);
/*******************************************/

int __io_putchar(int ch)
{
	debug_uart_write(ch);
	return ch;
}

void debug_uart_init(void)
{
	/*1.- Enable clock access for UART pins' GPIO port (GPIOA)*/
	RCC->AHB1ENR |= GPIOAEN;

	/*2.- Set PA2 mode to alternate function*/
	GPIOA->MODER &= ~(1U<<4);
	GPIOA->MODER |= (1U<<5);

	/*3.- Set PA3 mode to alternate function*/
	GPIOA->MODER &= ~(1U<<6);
	GPIOA->MODER |= (1U<<7);

	/*4.- Set PA2 alternate function type to UART2_TX (AF7) 0111: AF7*/
	/*GPIO alternate function low register (GPIOx_AFRL) (x = A..C and H)*/
	GPIOA->AFR[0] |=(1U<<8); //1
	GPIOA->AFR[0] |=(1U<<9); //1
	GPIOA->AFR[0] |=(1U<<10); //1
	GPIOA->AFR[0] &= ~(1U<<11); //0

	/*5.- Set PA3 alternate function type to UART2_RX (AF7)*/
	/*GPIO alternate function low register (GPIOx_AFRL) (x = A..C and H)*/
	GPIOA->AFR[0] |=(1U<<12);
	GPIOA->AFR[0] |=(1U<<13);
	GPIOA->AFR[0] |=(1U<<14);
	GPIOA->AFR[0] &= ~(1U<<15);

	/*Configure UART module*/

	/*6.- Enable clock access to the UART module*/
	/*USART2 is located on APB1 bus*/
	/*USART2EN is bit 17*/
	RCC->APB1ENR |= UART2EN;

	/*7.- Disable UART module*/
	/*Control Register 1 --> Bit 13 enables UART*/
	USART2->CR1 &=~CR1_UE;

	/*8.- Set UART baudrate*/
	USART2->BRR =  compute_uart_bd(SYS_FREQ,UART_BAUDRATE);

	/*9.- Set transfer direction */
	/*Cleans the current configuration of CR1 - Enables Transmitter and Receiver - CR1 Bit 3 and Bit 2 */
	USART2->CR1 = (CR1_TE | CR1_RE);

	/*10.- Enable UART module*/
	/*Keeps the last configuration and enables USART (|= Means that the bit mask before is not changed)*/
	USART2->CR1 |= CR1_UE;
}

static void debug_uart_write(int ch)
{
	/*Make sure the transmit data register is empty*/
	while(!(USART2->SR & SR_TXE)){}

	/*Write to transmit data register*/
	USART2->DR = (ch & 0xFF);
}

/*Configure the baudrate of the UART with peripheral clock and baudrate assigned
 *
 * Baudrate defined: 115200
 * System frequency: 16000000 --> For the board: STM32F410RB
 * Formula = peripheral clock + (baudrate / 2) / baudrate
 *
 * */
static uint16_t compute_uart_bd(uint32_t periph_clk,uint32_t baudrate)
{
	return ((periph_clk + (baudrate/2U))/baudrate);
}
