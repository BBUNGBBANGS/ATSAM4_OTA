#include "flash.h"  
#include "boot_uart.h"

Flash_Status_t Flash_Init(void)
{
    Flash_Status_t status = FLASH_OK;

    if (FLASH_RC_OK != flash_init(FLASH_ACCESS_MODE_128, 6))
    {
        status = FLASH_ERROR;
    }

	/* unlock whole flash section */
	if (FLASH_RC_OK != flash_unlock(0x00400000, 0x00420000 - 1, 0, 0))
    {
        status = FLASH_ERROR;        
    }

    return status;
}

/**
 * @brief   This function erases the memory.
 * @param   address: First address to be erased (the last is the end of the flash).
 * @return  status: Report about the success of the erasing.
 */
Flash_Status_t Flash_Erase(uint32_t address)
{
    Flash_Status_t status = FLASH_OK;

    if (FLASH_RC_OK != flash_erase_page(address, IFLASH_ERASE_PAGES_8))
    {
        status = FLASH_ERROR;
    }

    return status;
}

/**
 * @brief   This function flashes the memory.
 * @param   address: First address to be written to.
 * @param   *data:   Array of the data that we want to write.
 * @param   *length: Size of the array.
 * @return  status: Report about the success of the writing.
 */
Flash_Status_t Flash_Write(uint32_t address, uint32_t *data, uint32_t size)
{
    Flash_Status_t status = FLASH_OK;

    if (FLASH_RC_OK != flash_write(address, data, (FLASH_PAGE_SIZE * size), 0))
    {
        status = FLASH_ERROR;
    }

    return status;
}

/**
 * @brief   Actually jumps to the user application.
 * @param   void
 * @return  void
 */
void Flash_Jump_To_Application(void)
{
	const Jump_Application_t* vector_p = (Jump_Application_t*)FLASH_APP1_START_ADDRESS;
    NVIC_DisableIRQ(UART0_IRQn);
	/* let's do The Jump! */
    /* Jump, used asm to avoid stack optimization */
    asm("msr msp, %0; bx %1;" : : "r"(vector_p->stack_addr), "r"(vector_p->func_p));
}

void Flash_Copy_App2_To_App1(void)
{
    uint8_t buf[50] = {0,};
    uint32_t buffer[256];
    uint32_t app1_address = FLASH_APP1_START_ADDRESS;
    uint32_t app2_address = FLASH_APP2_START_ADDRESS;
    uint8_t percent = 0;

    for (uint32_t page = 0; page < 52; page++) 
    {
        // copy data from App2 to buffer
        for (uint32_t i = 0; i < 256; i++) 
        {
            buffer[i] = ((uint32_t *)app2_address)[i];
        }
        // erase App1 page
        if (0 == (page % 4))
        {
            Flash_Erase(app1_address);
        }
        // write buffer to App1 page
        Flash_Write(app1_address, buffer, 2);
        
        // update addresses and remaining copy size
        app1_address += FLASH_PAGE_SIZE * 2;
        app2_address += FLASH_PAGE_SIZE * 2;
        
        if (percent <= 100)
        {
            sprintf((char *)buf,"Firmware Copy : %d%% \r\n",percent);
            Uart_Transmit_Str(buf);
        }
        else
        {
            sprintf((char *)buf,"Firmware Update Finished.\r\n");
            Uart_Transmit_Str(buf);            
        }
        percent += 2;
    }
}