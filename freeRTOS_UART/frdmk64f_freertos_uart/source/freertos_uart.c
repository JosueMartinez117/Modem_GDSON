#include "FreeRTOS.h"
#include "task.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_uart.h"
#include "board.h"
#include "semphr.h"

#define TX_TASK_STACK_SIZE 512
#define RX_TASK_STACK_SIZE 512

#define TX_TASK_PRIORITY 4
#define RX_TASK_PRIORITY 3

#define UART_BAUDRATE 115200
#define UART_CLK_FREQ CLOCK_GetFreq(kCLOCK_CoreSysClk)
//kCLOCK_CoreSysClk
//kCLOCK_FlexBusClk

// Define the UART interrupt priority
#define UART0_IRQ_PRIO (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1)
//#define UART1_IRQ_PRIO (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1)

// Define the UART instance and buffer size
#define UART_INSTANCE UART0
//#define UART_INSTANCE UART1

#define BUFFER_SIZE 10

// Define the message to be sent
static volatile bool rxUartInterrupt = false;
static volatile bool txUartInterrupt = false;
const char *message = "ATI\r\n";
const char *message_2 = "AT+COPS?\r\n";
SemaphoreHandle_t xSemaphore;

// Define the receive buffer and index
uint8_t rx_buffer[BUFFER_SIZE];
volatile uint32_t rx_index = 0;

// UART TX task
static void tx_task(void *pvParameters) {

    while (1) {
    	if(txUartInterrupt){
            /* Wait for the semaphore to become available */
            xSemaphoreTake(xSemaphore, portMAX_DELAY);
    		// Send message through UART
            UART_WriteBlocking(UART_INSTANCE, (const uint8_t *)message, strlen(message));
            vTaskDelay(pdMS_TO_TICKS(1000));
            txUartInterrupt=false;
    	}

    }
}

// UART RX task
static void rx_task(void *pvParameters) {
    while (1) {
    	if(rxUartInterrupt){
        // Wait for UART receive interrupt
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Print received message
        PRINTF("Received message: %s\r\n", rx_buffer);
        rx_index = 0;
        rxUartInterrupt=false;
    	}
    }
}

// UART receive interrupt handler
//void UART1_RX_TX_IRQHandler(void)
void UART0_RX_TX_IRQHandler(void) {

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	uint32_t status_flags = UART_GetStatusFlags(UART_INSTANCE);

	// Check if the receive data register is full
    /*if ((kUART_RxDataRegFullFlag | kUART_RxOverrunFlag) & UART_GetStatusFlags(UART0)) {

    	  // Read the received byte
          uint8_t data = UART_ReadByte(UART_INSTANCE);

          // Check if the receive buffer is full
          if (rx_index >= BUFFER_SIZE - 1) {
              rx_index = 0;
          }

          // Add the received byte to the buffer
          rx_buffer[rx_index++] = data;

          // Notify the receive task
          vTaskNotifyGiveFromISR(NULL, pdTRUE);
          rxUartInterrupt = true;
    }*/


	/*
	 * checks if the transmit data register is empty by checking the
	 * UART_S1_TDRE_MASK flag in the UART status register.
	 *
	 * */
	 if (UART_INSTANCE->S1 & UART_S1_TDRE_MASK){
    	txUartInterrupt = true;

    	UART_ClearStatusFlags(UART_INSTANCE, kUART_TxDataRegEmptyInterruptEnable);
    	xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);

      	//UART_DisableInterrupts(UART1, kUART_TxDataRegEmptyInterruptEnable);
      	//DisableIRQ(UART1_RX_TX_IRQn);
    }

	 portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

int main(void) {
    // Initialize the board
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    // Initialize the UART
    uart_config_t uartConfig;
    UART_GetDefaultConfig(&uartConfig);
    uartConfig.baudRate_Bps = UART_BAUDRATE;
    UART_Init(UART_INSTANCE, &uartConfig, UART_CLK_FREQ);

    /*Create Semaphore*/
    xSemaphore = xSemaphoreCreateBinary();

    // Enable the UART interrupt
    UART_EnableInterrupts(UART_INSTANCE, kUART_TxDataRegEmptyInterruptEnable);
    NVIC_SetPriority(UART0_RX_TX_IRQn, UART0_IRQ_PRIO);
    NVIC_ClearPendingIRQ(UART0_RX_TX_IRQn);
    EnableIRQ(UART0_RX_TX_IRQn);

    //Interrupciones UART1
    /*UART_EnableInterrupts(UART_INSTANCE, kUART_TxDataRegEmptyInterruptEnable);
    NVIC_SetPriority(UART1_RX_TX_IRQn, UART1_IRQ_PRIO);
    NVIC_ClearPendingIRQ(UART1_RX_TX_IRQn);
    EnableIRQ(UART1_RX_TX_IRQn);*/


    // Create the TX task
    xTaskCreate(tx_task, "TX Task", TX_TASK_STACK_SIZE, NULL, TX_TASK_PRIORITY, NULL);

    // Create the RX task
    xTaskCreate(rx_task, "RX Task", RX_TASK_STACK_SIZE, NULL, RX_TASK_PRIORITY, NULL);


    // Start the scheduler
    vTaskStartScheduler();

    // Should not reach here
    return 0;
}
