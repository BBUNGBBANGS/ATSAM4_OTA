#ifndef _UART_H
#define _UART_H

#include "conf_uart_serial.h"

/* Status report for the functions. */
typedef enum 
{
    UART_OK     = 0x00u, /**< The action was successful. */
    UART_ERROR  = 0xFFu  /**< Generic error. */
} uart_status;

typedef struct 
{
    uint8_t header;
    uint8_t packet_num[2];
    uint8_t data[1024];
    uint8_t checksum[2];
} Uart_Packet_t;

typedef enum 
{
    UART_PACKET_HEADER = 0x00u,
    UART_PACKET_NUM,
    UART_PACKET_DATA,
    UART_PACKET_CHECKSUM,
} Uart_Packet_Index;

extern Uart_Packet_t Uart_Packet;
extern uint8_t Uart_Buffer[2048];
extern uint16_t Uart_Buffer_Length;
extern uint8_t Uart_Packet_Received_Flag;

extern void Boot_Uart_Init(void);
extern uart_status uart_transmit_str(uint8_t *data);
extern uart_status uart_transmit_ch(uint8_t data);
extern uart_status uart_receive(uint8_t *data, uint16_t length);
extern uart_status Uart_Packet_Receive(uint8_t *data, Uart_Packet_Index index);
#endif