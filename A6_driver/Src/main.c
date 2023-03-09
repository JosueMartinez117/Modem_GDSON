#include "a6_driver.h"
#include "a6_lib.h"

char user_phone_number[] = {"\"3318609739\""};
char sms_message[] = "Hello from remote STM32 Machine";

int main()
{
	/*A6 USER APIS*/
	a6_module_init();
	a6_send_sms(sms_message,user_phone_number);
	a6_dial_call(user_phone_number);
}
