
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "asf.h"
#include "conf_uart_serial.h"

uint32_t rx_data_length;
char rx_data_buffer[1024];

static void Rx_Data_Clear(char *buffer,uint32_t length);
static void configure_console(void);

int main(void)
{
	/* Initialize the board. */
	sysclk_init();
	board_init();

	/* Initialize the UART console. */
	configure_console();

	printf("Main function address: %p\n", &main);
	while(1)
	{
		if (rx_data_length != 0) 
		{
            printf("Received: %s\r\n", rx_data_buffer); // 수신된 문자 출력
			Rx_Data_Clear(rx_data_buffer,strlen(rx_data_buffer));
            rx_data_length = 0; // 문자 저장 변수 초기화
        }
	}
}

void UART1_Handler(void)
{
    if (usart_get_interrupt_mask(UART1) & US_IER_RXRDY) 
	{
        usart_getchar(UART1,&rx_data_buffer[rx_data_length]); // ?�신??문잝 ?�??
		rx_data_length++;
    }
}

static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = 
	{
		.baudrate =		CONF_UART_BAUDRATE,
		.charlength =	CONF_UART_CHAR_LENGTH,
		.paritytype =	CONF_UART_PARITY,
		.stopbits =		CONF_UART_STOP_BITS,
	};

	/* Configure UART console. */
	sysclk_enable_peripheral_clock(ID_UART1);
	stdio_serial_init(UART1, &uart_serial_options);
	usart_enable_interrupt(UART1, US_IER_RXRDY); // UART RX 인터럽트 활성화
    NVIC_EnableIRQ(UART1_IRQn); // 전체 인터럽트 활성화
}

static void Rx_Data_Clear(char *buffer,uint32_t length)
{
	for(uint32_t i = 0; i < length; i++)
	{
		*buffer = 0;
	}
}