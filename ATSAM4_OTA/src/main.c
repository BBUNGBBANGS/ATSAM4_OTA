#include "main.h"
#include "boot_uart.h"
#include "xmodem.h"

int main(void)
{
	/* Initialize the board. */
	sysclk_init();
	board_init();

	/* Initialize the UART console. */
	Boot_Uart_Init();

	printf("========================================\r\n");
	printf("=  ATSAM4S Firmware Update Bootloader  =\r\n");
	printf("========================================\r\n");

	while(1)
	{
		printf("Please send a new binary file with Xmodem protocol to update the firmware.\n\r");
		xmodem_receive();
    	printf("\n\rFailed... Please try again.\n\r");
	}
}
