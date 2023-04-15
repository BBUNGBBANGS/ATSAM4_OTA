#include "main.h"
#include "boot_uart.h"

volatile uint8_t Uart_Rx_Buf[2048];
Uart_Packet_t Uart_Packet;
uint16_t Uart_Buffer_Length;
uint8_t Uart_Packet_Received_Flag;

void Boot_Uart_Init(void)
{
	const usart_serial_options_t uart_serial_options = 
	{
		.baudrate = 230400,
		.charlength = CONF_UART_CHAR_LENGTH,
		.paritytype = CONF_UART_PARITY,
		.stopbits =	CONF_UART_STOP_BITS,
	};

	/* Configure UART console. */
	sysclk_enable_peripheral_clock(ID_UART1);
	stdio_serial_init(UART1, &uart_serial_options);
	usart_enable_interrupt(UART1, US_IER_RXRDY); // UART RX 인터럽트 활성화
    NVIC_EnableIRQ(UART1_IRQn); // 전체 인터럽트 활성화
}

/**
 * @brief   Transmits a single char to UART.
 * @param   *data: The char.
 * @return  status: Report about the success of the transmission.
 */
Uart_Status_t Uart_Transmit(uint8_t data)
{
	Uart_Status_t status = UART_ERROR;

	if(uart_write(UART1,data) == 0)
	{
		status = UART_OK;
	}

	return status;
}

/**
 * @brief   Transmits a string to UART.
 * @param   *data: Array of the data.
 * @return  status: Report about the success of the transmission.
 */
Uart_Status_t Uart_Transmit_Str(uint8_t *data)
{
	Uart_Status_t status = UART_ERROR;
	uint16_t length = (uint16_t)strlen((char *)data);

	/* Calculate the length. */
	for(uint16_t i = 0; i < length; i++)
	{
		status = Uart_Transmit(*(data+i));
		delay_us(100);
		if (status == UART_ERROR)
		{
			return status;
		}
	}

	status = UART_OK;

	return status;
}

Uart_Status_t Uart_Packet_Receive(uint8_t *data, Uart_Packet_Index_t index)
{
	Uart_Status_t status = UART_ERROR;
	if ((Uart_Buffer_Length > 0) || (Uart_Packet_Received_Flag == 1))
	{
		switch(index)
		{
			case UART_PACKET_HEADER : 
				*data = Uart_Packet.header;
			break;
			case UART_PACKET_NUM :
				for(uint8_t i = 0; i < 2; i++)
				{
					*(data+i) = Uart_Packet.packet_num[i];
				}
			break;
			case UART_PACKET_DATA :
				for(uint16_t i = 0; i < 1024; i++)
				{
					*(data+i) = Uart_Packet.data[i];
				}
			break;
			case UART_PACKET_CHECKSUM :
				for(uint8_t i = 0; i < 2; i++)
				{
					*(data+i) = Uart_Packet.checksum[i];
				}		
			break;
		}
		status = UART_OK;
	}

	return status;
}

void UART1_Handler(void)
{
	uint8_t rx_data = 0;
    if (usart_get_interrupt_mask(UART1) & US_IER_RXRDY) 
	{
        usart_getchar(UART1,(uint32_t *)&Uart_Rx_Buf[Uart_Buffer_Length]); 
		if (Uart_Buffer_Length < 1)
		{
			Uart_Packet.header = Uart_Rx_Buf[Uart_Buffer_Length];
		}
		else if (Uart_Buffer_Length < 3)
		{
			Uart_Packet.packet_num[Uart_Buffer_Length - 1] = Uart_Rx_Buf[Uart_Buffer_Length];
		}
		else if (Uart_Buffer_Length < 1027)
		{
			Uart_Packet.data[Uart_Buffer_Length - 3] = Uart_Rx_Buf[Uart_Buffer_Length];
		}
		else if (Uart_Buffer_Length < 1029)
		{
			Uart_Packet.checksum[Uart_Buffer_Length - 1027] = Uart_Rx_Buf[Uart_Buffer_Length];
		}
		
		if(Uart_Buffer_Length >= 1028)
		{
			Uart_Packet_Received_Flag = 1;
		}
		Uart_Buffer_Length++;
    }
}

