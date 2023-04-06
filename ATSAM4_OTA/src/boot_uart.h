#ifndef _UART_H
#define _UART_H

#include "conf_uart_serial.h"

/* Status report for the functions. */
typedef enum 
{
    UART_OK     = 0x00u, /**< The action was successful. */
    UART_ERROR  = 0xFFu  /**< Generic error. */
} uart_status;

extern uint8_t Uart_Buffer[1029];
extern uint16_t Uart_Buffer_Length;
extern uint8_t Uart_Packet_Received_Flag;

extern void Boot_Uart_Init(void);
extern uart_status uart_transmit_str(uint8_t *data);
extern uart_status uart_transmit_ch(uint8_t data);
extern uart_status uart_receive(uint8_t *data, uint16_t length);
#endif