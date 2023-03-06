#include "a6_driver.h"

int main()
{
	debug_uart_init();
	while(1){
		printf("Hello from printf \n\r");
		for(int i=0; i<9000; i++){}
	}
}
