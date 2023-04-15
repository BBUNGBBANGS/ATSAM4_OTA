#include "main.h"
#include "boot_uart.h"
#include "xmodem.h"
#include "flash.h"

volatile uint32_t * const Reprogram_Info = (uint32_t *)FLASH_REPROGRAM_STATUS_ADDRESS;

int main(void)
{
	uint8_t buf[100] = {0,};
	uint32_t Reprogram_Buffer[128] = {0,};
	uint32_t Start_Address_Data = 0;
	uint32_t Copy_Address_Data = 0;
	uint32_t Reprogram_Info_Data = *Reprogram_Info;
	/* Initialize the board. */
	sysclk_init();
	board_init();
	Flash_Init();
	Boot_Uart_Init();
	delay_ms(10);
	Start_Address_Data = *(uint32_t *)FLASH_APP1_START_ADDRESS;
	Copy_Address_Data = *(uint32_t *)FLASH_APP2_START_ADDRESS;		
	#ifdef _DEBUG
	sprintf((char *)buf,"Start Address Data : %x\r\n",Start_Address_Data);
	Uart_Transmit_Str(buf);
	sprintf((char *)buf,"Copy Address Data : %x\r\n",Copy_Address_Data);
	Uart_Transmit_Str(buf);		
	sprintf((char *)buf,"Reprogram Info Data : %x\r\n",Reprogram_Info_Data);
	Uart_Transmit_Str(buf);
	#endif
	if ((Start_Address_Data != 0xFFFFFFFFu) && (Start_Address_Data != 0x00000000u) && (Reprogram_Info_Data != 0x00000001))
	{
		Flash_Jump_To_Application();
	}

	sprintf((char *)buf,"\r\n========================================\r\n");
	Uart_Transmit_Str(buf);
	sprintf((char *)buf,"=  ATSAM4S Firmware Update Bootloader  =\r\n");
	Uart_Transmit_Str(buf);
	sprintf((char *)buf,"========================================\r\n");
	Uart_Transmit_Str(buf);

	Flash_Erase(FLASH_REPROGRAM_STATUS_ADDRESS);
	Flash_Write(FLASH_REPROGRAM_STATUS_ADDRESS, Reprogram_Buffer, 1);

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

