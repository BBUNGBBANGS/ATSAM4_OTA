﻿#include "main.h"
#include "boot_uart.h"
#include "xmodem.h"
#include "flash.h"

volatile uint32_t * const Reprogram_Info = (uint32_t *)0x0041FFF8;
int main(void)
{
	uint8_t buf[100] = {0,};
	uint32_t Reprogram_Buffer[2] = {0,};
	uint32_t *Application_Address = (uint32_t *)FLASH_APP1_START_ADDRESS;
	uint32_t temp;
	/* Initialize the board. */
	sysclk_init();
	board_init();
	Flash_Init();
	Boot_Uart_Init();
	delay_ms(10);
	temp = *Application_Address;
	uint32_t Reprogram_Info_Data = *Reprogram_Info;
	if ((*Application_Address != 0xFFFFFFFFu) && (*Application_Address != 0x00000000u) && (Reprogram_Info_Data != 0x01))
	{
		Flash_Jump_To_Application();
	}
	sprintf((char *)buf,"\r\n========================================\r\n");
	Uart_Transmit_Str(buf);
	sprintf((char *)buf,"=  ATSAM4S Firmware Update Bootloader  =\r\n");
	Uart_Transmit_Str(buf);
	sprintf((char *)buf,"========================================\r\n");
	Uart_Transmit_Str(buf);
	sprintf((char *)buf,"\r\n\r\nErase App2 Flash Section\r\n");
	Uart_Transmit_Str(buf);
	Flash_Erase(FLASH_APP2_START_ADDRESS,104);
	flash_unlock(0x0041FFF8, 0x0041FFF8 + 8 - 1, 0, 0);
	delay_ms(10);
	flash_write(0x0041FFF8, Reprogram_Buffer, 8, 0);
	delay_ms(10);
	flash_lock(0x0041FFF8, 0x0041FFF8 + 8 - 1, 0, 0);
	delay_ms(10);
	while(1)
	{
		sprintf((char *)buf,"Please send a new binary file with Xmodem protocol to update the firmware.\r\n");
		Uart_Transmit_Str(buf);
		xmodem_receive();		
		sprintf((char *)buf,"\r\nFailed... Please try again.\r\n");
		Uart_Transmit_Str(buf);
		Flash_Jump_To_Application();
	}
}
