#include "main.h"
#include "boot_uart.h"
#include "xmodem.h"

int main(void)
{
	uint8_t buf[100] = {0,};

	/* Initialize the board. */
	sysclk_init();
	board_init();

	/* Initialize the UART console. */
	Boot_Uart_Init();
	delay_ms(100);
	sprintf((char *)buf,"========================================\r\n");
	Uart_Transmit_Str(buf);
	sprintf((char *)buf,"=  ATSAM4S Firmware Update Bootloader  =\r\n");
	Uart_Transmit_Str(buf);
	sprintf((char *)buf,"========================================\r\n");
	Uart_Transmit_Str(buf);
	while(1)
	{
		sprintf((char *)buf,"Please send a new binary file with Xmodem protocol to update the firmware.\r\n");
		Uart_Transmit_Str(buf);
		xmodem_receive();		
		sprintf((char *)buf,"\r\nFailed... Please try again.\r\n");
		Uart_Transmit_Str(buf);
	}
}

void Jump_To_Application(const uint32_t address)
{
	const Jump_Application_t* vector_p = (Jump_Application_t*)address;

	/* let's do The Jump! */
    /* Jump, used asm to avoid stack optimization */
    asm("msr msp, %0; bx %1;" : : "r"(vector_p->stack_addr), "r"(vector_p->func_p));
}