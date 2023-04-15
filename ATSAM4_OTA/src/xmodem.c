#include "main.h"
#include "xmodem.h"
#include "boot_uart.h"
#include "flash.h"

/* Global variables. */
static uint8_t xmodem_packet_number = 1u;         /**< Packet number counter. */
static uint32_t xmodem_actual_flash_address = 0u; /**< Address where we have to write. */
static uint8_t x_first_packet_received = false;   /**< First packet or not. */

/* Local functions. */
static uint16_t xmodem_calc_crc(uint8_t *data, uint16_t length);
static xmodem_status xmodem_handle_packet(uint8_t size);
static xmodem_status xmodem_error_handler(uint8_t *error_number, uint8_t max_error_number);

/**
 * @brief   This function is the base of the Xmodem protocol.
 *          When we receive a header from UART, it decides what action it shall take.
 * @param   void
 * @return  void
 */

void xmodem_receive(void)
{
    volatile xmodem_status status = X_OK;
    uint8_t error_number = 0u;

    x_first_packet_received = false;
    xmodem_packet_number = 1u;
    xmodem_actual_flash_address = FLASH_APP2_START_ADDRESS;

    /* Loop until there isn't any error (or until we jump to the user application). */
    while (X_OK == status)
    {
        uint8_t header = 0x00u;

        /* Get the header from UART. */
		Uart_Status_t comm_status = Uart_Packet_Receive(&header, UART_PACKET_HEADER);
        if ((header == X_SOH) || (header == X_STX))
        {
            x_first_packet_received = true;
        }
        /* Spam the host (until we receive something) with ACSII "C", to notify it, we want to use CRC-16. */
        if (x_first_packet_received == false)
        {
            (void)Uart_Transmit(X_C);
            delay_ms(10);
        }
		
        /* The header can be: SOH, STX, EOT and CAN. */
        switch(header)
        {
            xmodem_status packet_status = X_ERROR;
            /* 128 or 1024 bytes of data. */
            case X_SOH:
            case X_STX:
                if(Uart_Packet_Received_Flag == 1)
                {
                    /* If the handling was successful, then send an ACK. */
                    packet_status = xmodem_handle_packet(header);
                    if (X_OK == packet_status)
                    {
                        (void)Uart_Transmit(X_ACK);
                    }
                    /* If the error was flash related, then immediately set the error counter to max (graceful abort). */
                    else if (X_ERROR_FLASH == packet_status)
                    {
                        error_number = X_MAX_ERRORS;
                        status = xmodem_error_handler(&error_number, X_MAX_ERRORS);
                    }
                    /* Error while processing the packet, either send a NAK or do graceful abort. */
                    else
                    {
                        status = xmodem_error_handler(&error_number, X_MAX_ERRORS);
                    }
                }
            break;
            /* End of Transmission. */
            case X_EOT:
            {
                uint8_t buf[100] = {0,};
                /* ACK, feedback to user (as a text), then jump to user application. */
                (void)Uart_Transmit(X_ACK);	
				delay_ms(10);
                sprintf((char *)buf,"\r\n\r\nFirmware download completed.\r\n");
		        Uart_Transmit_Str(buf);
                sprintf((char *)buf,"Please wait, copying application software.\r\n");
		        Uart_Transmit_Str(buf);
                Flash_Copy_App2_To_App1();
				NVIC_SystemReset();         
            }
            break;
            /* Abort from host. */
            case X_CAN:
                status = X_ERROR;
            break;
            default:
                /* Wrong header. */
                if (UART_OK == comm_status)
                {
                    status = xmodem_error_handler(&error_number, X_MAX_ERRORS);
                }
            break;
        }
    }
}

static uint16_t xmodem_calc_crc(uint8_t *data, uint16_t length)
{
    uint16_t crc = 0u;
    while (length)
    {
        length--;
        crc = crc ^ ((uint16_t)*data++ << 8u);
        for (uint8_t i = 0u; i < 8u; i++)
        {
            if (crc & 0x8000u)
            {
                crc = (crc << 1u) ^ 0x1021u;
            }
            else
            {
                crc = crc << 1u;
            }
        }
    }
    return crc;
}

/**
 * @brief   This function handles the data packet we get from the xmodem protocol.
 * @param   header: SOH or STX.
 * @return  status: Report about the packet.
 */
static xmodem_status xmodem_handle_packet(uint8_t header)
{
    xmodem_status status = X_OK;
    uint16_t size = 0u;

    /* 2 bytes for packet number, 1024 for data, 2 for CRC*/
    uint8_t received_packet_number[X_PACKET_NUMBER_SIZE];
    uint8_t received_packet_data[X_PACKET_1024_SIZE];
    uint8_t received_packet_crc[X_PACKET_CRC_SIZE];

    /* Get the size of the data. */
    if (X_SOH == header)
    {
        size = X_PACKET_128_SIZE;
    }
    else if (X_STX == header)
    {
        size = X_PACKET_1024_SIZE;
    }
    else
    {
        /* Wrong header type. This shoudn't be possible... */
        status |= X_ERROR;
    }

    Uart_Status_t comm_status = UART_OK;
    /* Get the packet number, data and CRC from UART. */
    comm_status |= Uart_Packet_Receive(&received_packet_number[0u], UART_PACKET_NUM);
    comm_status |= Uart_Packet_Receive(&received_packet_data[0u], UART_PACKET_DATA);
    comm_status |= Uart_Packet_Receive(&received_packet_crc[0u], UART_PACKET_CHECKSUM);
    
    /* Merge the two bytes of CRC. */
    uint16_t crc_received = ((uint16_t)received_packet_crc[X_PACKET_CRC_HIGH_INDEX] << 8u) | ((uint16_t)received_packet_crc[X_PACKET_CRC_LOW_INDEX]);
    /* We calculate it too. */
    uint16_t crc_calculated = xmodem_calc_crc(&received_packet_data[0u], size);
    
    /* Communication error. */
    if (UART_OK != comm_status)
    {
        status |= X_ERROR_UART;
    }

    /* Error handling and flashing. */
    if (X_OK == status)
    {
        if (xmodem_packet_number != received_packet_number[0u])
        {
            /* Packet number counter mismatch. */
            status |= X_ERROR_NUMBER;
        }
        if (255u != (received_packet_number[X_PACKET_NUMBER_INDEX] + received_packet_number[X_PACKET_NUMBER_COMPLEMENT_INDEX]))
        {
            /* The sum of the packet number and packet number complement aren't 255. */
            /* The sum always has to be 255. */
            status |= X_ERROR_NUMBER;
        }
        if (crc_calculated != crc_received)
        {
            /* The calculated and received CRC are different. */
            status |= X_ERROR_CRC;
        }
    }

    if (1 == (xmodem_packet_number % 4))
    {
        Flash_Erase(xmodem_actual_flash_address);
    }

    /* Do the actual flashing (if there weren't any errors). */
    if ((X_OK == status) && (FLASH_OK != Flash_Write(xmodem_actual_flash_address, (uint32_t*)&received_packet_data[0u], (uint32_t)size/FLASH_PAGE_SIZE)))
    {
        /* Flashing error. */
        status |= X_ERROR_FLASH;
    }

    /* Raise the packet number and the address counters (if there weren't any errors). */
    if (X_OK == status)
    {
        xmodem_packet_number++;
        xmodem_actual_flash_address += size;
    }
    
    Uart_Buffer_Length = 0;
	Uart_Packet_Received_Flag = 0;
    return status;
}

/**
 * @brief   Handles the xmodem error.
 *          Raises the error counter, then if the number of the errors reached critical, do a graceful abort, otherwise send a NAK.
 * @param   *error_number:    Number of current errors (passed as a pointer).
 * @param   max_error_number: Maximal allowed number of errors.
 * @return  status: X_ERROR in case of too many errors, X_OK otherwise.
 */
static xmodem_status xmodem_error_handler(uint8_t *error_number, uint8_t max_error_number)
{
    xmodem_status status = X_OK;
    /* Raise the error counter. */
    (*error_number)++;
    /* If the counter reached the max value, then abort. */
    if ((*error_number) >= max_error_number)
    {
        /* Graceful abort. */
        (void)Uart_Transmit(X_CAN);
        (void)Uart_Transmit(X_CAN);
        status = X_ERROR;
    }
    /* Otherwise send a NAK for a repeat. */
    else
    {
        (void)Uart_Transmit(X_NAK);
        status = X_OK;
    }
    return status;
}