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
	delay_ms(100);
	//uart_transmit_str("========================================\r\n");
	//uart_transmit_str("=  ATSAM4S Firmware Update Bootloader  =\r\n");
	//uart_transmit_str("========================================\r\n");
	uart_transmit_ch('h');
	uart_transmit_ch('i');
	uart_transmit_str("hi\r\n");
	printf("hello world\r\n");
	while(1)
	{
		//uart_transmit_str("Please send a new binary file with Xmodem protocol to update the firmware.\n\r");
		//xmodem_receive();
    	//uart_transmit_str("\n\rFailed... Please try again.\n\r");
		delay_ms(100);
	}
}
