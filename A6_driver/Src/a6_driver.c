#include "a6_driver.h"

/*Defines*/
#define GPIOAEN			(1U<<0) // 0B 0000 0000 0000 0000 0000 0000 0000 0001 /*RCC AHB1 RCC_AHB1ENR - Peripheral clock enable register*/
#define UART2EN			(1U<<17)
#define UART1EN			(1U<<4)
#define CR1_UE			(1U<<13)
#define CR1_RE			(1U<<2)
#define CR1_TE			(1U<<3)
#define SR_TXE			(1U<<7)
#define CR1_RXNEIE		(1U<<5)
#define UART_BAUDRATE	115200
#define SYS_FREQ		16000000 //1 Second

/*Systick for Cortex-M4*/
#define CTRL_ENABLE		(1U<<0)
#define CTRL_CLKSRC		(1U<<2)
#define CTRL_COUNTFLAG	(1U<<16)
#define SYSTICK_LOAD_VAL 16000 //1 ms

/*******************************************/

/*Prototypes*/
void debug_uart_init(void);
static uint16_t compute_uart_bd(uint32_t periph_clk,uint32_t baudrate);
static void debug_uart_write(int ch);
void a6_uart_init(void);
void a6_uart_write_char(char ch);
void systick_delay_ms(uint32_t delay);
/*******************************************/

int __io_putchar(int ch)
{
	debug_uart_write(ch);
	return ch;
}

void debug_uart_init(void)
{
	/*1.- Enable clock access for UART pins' GPIO port (GPIOA)*/
	/*RCC AHB1 peripheral clock enable register (RCC_AHB1ENR)*/
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

/*
 *
 * UART Module	: USART 1 - BUS APB2
 * UART Pins	: PA9 = TX, PA10 = RX
 * A6 TX Pin 	: PA10 (RX)
 * A6 RX Pin	: PA9  (TX)
 *
 */

void a6_uart_init(void){

	/*1.- Enable clock access for UART pins' GPIO port (GPIOA)*/
	RCC->AHB1ENR |= GPIOAEN;

	/*2.- Set PA9 mode to alternate function*/
	GPIOA->MODER &= ~(1U<<18);
	GPIOA->MODER |= (1U<<19);

	/*3.- Set PA10 mode to alternate function*/
	GPIOA->MODER &= ~(1U<<20);
	GPIOA->MODER |= (1U<<21);

	/*4.- Set PA9 alternate function type to UART1_TX (AF7) 0111: AF7*/
	/*GPIO alternate function low register (GPIOx_AFRL) (x = A..C and H)*/
	GPIOA->AFR[1] |=(1U<<4); //1
	GPIOA->AFR[1] |=(1U<<5); //1
	GPIOA->AFR[1] |=(1U<<6); //1
	GPIOA->AFR[1] &= ~(1U<<7); //0

	/*5.- Set PA10 alternate function type to UART1_RX (AF7)*/
	/*GPIO alternate function low register (GPIOx_AFRL) (x = A..C and H)*/
	GPIOA->AFR[1] |=(1U<<8);
	GPIOA->AFR[1] |=(1U<<9);
	GPIOA->AFR[1] |=(1U<<10);
	GPIOA->AFR[1] &= ~(1U<<11);

	/*6.- Enable clock access to the UART module*/
	/*USART1 is located on APB2 bus*/
	/*USART1EN is bit 4*/
	RCC->APB2ENR |= UART1EN;

	/*7.- Disable UART module*/
	/*Control Register 1 --> Bit 13 enables UART*/
	USART1->CR1 &=~CR1_UE;

	/*8.- Set UART baudrate*/
	USART1->BRR =  compute_uart_bd(SYS_FREQ,UART_BAUDRATE);

	/*9.- Set transfer direction */
	/*Cleans the current configuration of CR1 - Enables Transmitter and Receiver - CR1 Bit 3 and Bit 2 */
	USART1->CR1 = (CR1_TE | CR1_RE);

	/*10.- Enable RXNEIE Interrupt*/
	USART1->CR1 |= CR1_RXNEIE;

	/*11.- Enable UART module*/
	/*Keeps the last configuration and enables USART (|= Means that the bit mask before is not changed)*/
	USART1->CR1 |= CR1_UE;


}

void a6_uart_write_char(char ch)
{
	/*Make sure the transmit data register is empty*/
	while(!(USART1->SR & SR_TXE)){}

	/*Write to transmit data register*/
	USART1->DR = (ch & 0xFF);

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


void systick_delay_ms(uint32_t delay)
{

	/*1.- Reload with number of clocks per millisecond*/
	SysTick->LOAD = SYSTICK_LOAD_VAL;

	/*2.- Clear systick current value register*/
	SysTick->VAL = 0;

	/*3.- Enable systick and select internal clock source*/
	SysTick->CTRL = (CTRL_CLKSRC | CTRL_ENABLE);

	for(int i = 0; i < delay; i++){
		while((SysTick->CTRL & CTRL_COUNTFLAG) == 0){}
	}

	SysTick->CTRL = 0;
}
