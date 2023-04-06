#ifndef _BOOT_UART_H
#define _BOOT_UART_H

#include "conf_uart_serial.h"

/* Status report for the functions. */
typedef enum 
{
    UART_OK     = 0x00u, /**< The action was successful. */
    UART_ERROR  = 0xFFu  /**< Generic error. */
} Uart_Status_t;

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
} Uart_Packet_Index_t;

extern Uart_Packet_t Uart_Packet;
extern uint16_t Uart_Buffer_Length;
extern uint8_t Uart_Packet_Received_Flag;

extern void Boot_Uart_Init(void);
extern Uart_Status_t Uart_Transmit(uint8_t data);
extern Uart_Status_t Uart_Transmit_Str(uint8_t *data);
extern Uart_Status_t Uart_Packet_Receive(uint8_t *data, Uart_Packet_Index_t index);
#endif