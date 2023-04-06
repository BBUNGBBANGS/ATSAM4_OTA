#include "main.h"
#include "boot_uart.h"

uint8_t Uart_Buffer[2048];
Uart_Packet_t Uart_Packet;
uint16_t Uart_Buffer_Length;
uint8_t Uart_Packet_Received_Flag;

static void Uart_Packet_Parsing(uint8_t data);

void Boot_Uart_Init(void)
{
	const usart_serial_options_t uart_serial_options = 
	{
		.baudrate = 115200,
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
	const usart_serial_options_t uart_serial_options = 
	{
		.baudrate =		CONF_UART_BAUDRATE,
		.charlength =	CONF_UART_CHAR_LENGTH,
		.paritytype =	CONF_UART_PARITY,
		.stopbits =		CONF_UART_STOP_BITS,
	};

/**
 * @brief   Transmits a single char to UART.
 * @param   *data: The char.
 * @return  status: Report about the success of the transmission.
 */
uart_status uart_transmit_ch(uint8_t data)
{
	uart_status status = UART_ERROR;

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
uart_status uart_transmit_str(uint8_t *data)
{
	uart_status status = UART_ERROR;

	/* Calculate the length. */
	for(uint16_t i = 0; i < strlen(data); i++)
	{
		if (UART_ERROR == uart_transmit_ch(*(data+i)))
		{
			return status;
		}
	}

	status = UART_OK;

	return status;
}
/**
 * @brief   Receives data from UART.
 * @param   *data: Array to save the received data.
 * @param   length:  Size of the data.
 * @return  status: Report about the success of the receiving.
 */
uart_status uart_receive(uint8_t *data, uint16_t length)
{
	uart_status status = UART_ERROR;

	if(Uart_Buffer_Length > 0)
	{
		for(uint16_t i = 0; i < length; i++)
		{
			*(data+i) = Uart_Buffer[i];
		}
		status = UART_OK;
	}
	else
	{
		status = UART_ERROR;
	}

	return status;
}

uart_status Uart_Packet_Receive(uint8_t *data, Uart_Packet_Index index)
{
	uart_status status = UART_ERROR;
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
    if (usart_get_interrupt_mask(UART1) & US_IER_RXRDY) 
	{
        usart_getchar(UART1,(uint32_t *)&Uart_Buffer[Uart_Buffer_Length]); 
		if (Uart_Buffer_Length < 1)
		{
			Uart_Packet.header = Uart_Buffer[Uart_Buffer_Length];
		}
		else if (Uart_Buffer_Length < 3)
		{
			Uart_Packet.packet_num[Uart_Buffer_Length - 1] = Uart_Buffer[Uart_Buffer_Length];
		}
		else if (Uart_Buffer_Length < 1027)
		{
			Uart_Packet.data[Uart_Buffer_Length - 3] = Uart_Buffer[Uart_Buffer_Length];
		}
		else if (Uart_Buffer_Length < 1029)
		{
			Uart_Packet.checksum[Uart_Buffer_Length - 1027] = Uart_Buffer[Uart_Buffer_Length];
		}
		
		if(Uart_Buffer_Length >= 1028)
		{
			Uart_Packet_Received_Flag = 1;
		}
		Uart_Buffer_Length++;
    }
}

